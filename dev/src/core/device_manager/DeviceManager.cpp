#include "core/device_manager/DeviceManager.hpp"

// clang-format off
#include <windows.h>
#include <setupapi.h>
// clang-format on

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <string>
#include <system_error>
#include <vector>

#include "core/device_manager/DeviceSubscriber.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace device_manager {

namespace {

constexpr std::uint16_t kStMicroelectronicsVendorId = 0x0483;

constexpr DWORD kReadIntervalTimeoutMs = 50;
constexpr DWORD kReadTotalTimeoutMs = 1000;
constexpr DWORD kWriteTimeoutMs = 1000;
constexpr DWORD kDriverBufferSize = 4096;
constexpr std::size_t kReadChunkSize = 1024;

utils::ErrorCode ParseVendorId(std::uint16_t* vendor_id,
                               const std::string& hardware_id) {
  const std::size_t position = hardware_id.find("VID_");
  if (vendor_id == nullptr || position == std::string::npos ||
      hardware_id.size() < position + 8) {
    return utils::ErrorCode::kInvalidArgument;
  }

  const std::string digits = hardware_id.substr(position + 4, 4);
  const char* last = digits.c_str() + digits.size();
  const auto result = std::from_chars(digits.c_str(), last, *vendor_id, 16);
  if (result.ec != std::errc() || result.ptr != last) {
    return utils::ErrorCode::kInvalidArgument;
  }
  return utils::ErrorCode::kOk;
}

std::string GetDeviceProperty(HDEVINFO device_info,
                              SP_DEVINFO_DATA* device_data, DWORD property) {
  std::array<char, 256> buffer{};
  if (SetupDiGetDeviceRegistryPropertyA(
          device_info, device_data, property, nullptr,
          reinterpret_cast<PBYTE>(buffer.data()),
          static_cast<DWORD>(buffer.size() - 1), nullptr) == 0) {
    return {};
  }
  return {buffer.data()};
}

std::string GetPortName(HDEVINFO device_info, SP_DEVINFO_DATA* device_data) {
  const HKEY key = SetupDiOpenDevRegKey(
      device_info, device_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
  if (key == INVALID_HANDLE_VALUE) {
    return {};
  }

  std::array<char, 256> name{};
  auto size = static_cast<DWORD>(name.size() - 1);
  DWORD type = 0;
  const LONG result =
      RegQueryValueExA(key, "PortName", nullptr, &type,
                       reinterpret_cast<LPBYTE>(name.data()), &size);
  RegCloseKey(key);
  if (result != ERROR_SUCCESS || type != REG_SZ) {
    return {};
  }
  return {name.data()};
}

utils::ErrorCode ListSerialPorts(std::vector<DeviceState>* devices) {
  GUID ports_guid{};
  DWORD guid_count = 0;
  if (SetupDiClassGuidsFromNameA("Ports", &ports_guid, 1, &guid_count) == 0 ||
      guid_count == 0) {
    return utils::ErrorCode::kPortScanFailed;
  }

  const HDEVINFO device_info =
      SetupDiGetClassDevsA(&ports_guid, nullptr, nullptr, DIGCF_PRESENT);
  if (device_info == INVALID_HANDLE_VALUE) {
    return utils::ErrorCode::kPortScanFailed;
  }

  SP_DEVINFO_DATA device_data{};
  device_data.cbSize = sizeof(device_data);
  for (DWORD index = 0;
       SetupDiEnumDeviceInfo(device_info, index, &device_data) != 0; ++index) {
    DeviceState device;
    device.port_name = GetPortName(device_info, &device_data);
    if (device.port_name.rfind("COM", 0) != 0) {
      continue;
    }

    device.device_name =
        GetDeviceProperty(device_info, &device_data, SPDRP_DEVICEDESC);
    if (device.device_name.empty()) {
      device.device_name = device.port_name;
    }

    std::uint16_t vendor_id = 0;
    const std::string hardware_id =
        GetDeviceProperty(device_info, &device_data, SPDRP_HARDWAREID);
    device.supported = utils::IsOk(ParseVendorId(&vendor_id, hardware_id)) &&
                       vendor_id == kStMicroelectronicsVendorId;
    device.connection_state = ConnectionState::kConnected;
    devices->push_back(device);
  }

  SetupDiDestroyDeviceInfoList(device_info);
  return utils::ErrorCode::kOk;
}

utils::ErrorCode ConfigurePort(HANDLE handle, BaudRate rate) {
  DCB dcb{};
  dcb.DCBlength = sizeof(dcb);
  if (GetCommState(handle, &dcb) == 0) {
    return utils::ErrorCode::kPortConfigFailed;
  }
  dcb.BaudRate = static_cast<DWORD>(rate);
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fBinary = TRUE;
  dcb.fParity = FALSE;
  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDtrControl = DTR_CONTROL_ENABLE;
  dcb.fRtsControl = RTS_CONTROL_ENABLE;
  dcb.fOutX = FALSE;
  dcb.fInX = FALSE;
  if (SetCommState(handle, &dcb) == 0) {
    return utils::ErrorCode::kPortConfigFailed;
  }

  COMMTIMEOUTS timeouts{};
  timeouts.ReadIntervalTimeout = kReadIntervalTimeoutMs;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant = kReadTotalTimeoutMs;
  timeouts.WriteTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = kWriteTimeoutMs;
  if (SetCommTimeouts(handle, &timeouts) == 0) {
    return utils::ErrorCode::kPortConfigFailed;
  }

  SetupComm(handle, kDriverBufferSize, kDriverBufferSize);
  PurgeComm(handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
  return utils::ErrorCode::kOk;
}

}  // namespace

DeviceManager::~DeviceManager() {
  for (const auto& entry : open_ports_) {
    CloseHandle(static_cast<HANDLE>(entry.second));
  }
}

utils::ErrorCode DeviceManager::Subscribe(IDeviceSubscriber* subscriber) {
  if (subscriber == nullptr) {
    return utils::ErrorCode::kInvalidArgument;
  }
  if (std::find(subscribers_.begin(), subscribers_.end(), subscriber) ==
      subscribers_.end()) {
    subscribers_.push_back(subscriber);
  }
  return utils::ErrorCode::kOk;
}

void DeviceManager::Unsubscribe(IDeviceSubscriber* subscriber) {
  subscribers_.erase(
      std::remove(subscribers_.begin(), subscribers_.end(), subscriber),
      subscribers_.end());
}

void DeviceManager::NotifySubscribers(DeviceEvent event,
                                      const DeviceState& device) {
  const std::vector<IDeviceSubscriber*> snapshot = subscribers_;
  for (IDeviceSubscriber* subscriber : snapshot) {
    if (std::find(subscribers_.begin(), subscribers_.end(), subscriber) !=
        subscribers_.end()) {
      subscriber->Update(event, device);
    }
  }
}

utils::ErrorCode DeviceManager::FindOpenPort(DeviceRole role,
                                             void** handle) const {
  if (role == DeviceRole::kNone || handle == nullptr) {
    return utils::ErrorCode::kInvalidArgument;
  }
  const auto device = std::find_if(
      devices_.begin(), devices_.end(),
      [role](const auto& entry) { return entry.second.role == role; });
  if (device == devices_.end()) {
    return utils::ErrorCode::kRoleNotAssigned;
  }
  const auto port = open_ports_.find(device->first);
  if (port == open_ports_.end()) {
    return utils::ErrorCode::kPortNotOpen;
  }
  *handle = port->second;
  return utils::ErrorCode::kOk;
}

utils::ErrorCode DeviceManager::ScanPorts() {
  std::vector<DeviceState> found;
  const utils::ErrorCode result = ListSerialPorts(&found);
  if (!utils::IsOk(result)) {
    return result;
  }

  for (const DeviceState& device : found) {
    UpdateDevice(device);
  }

  for (auto& entry : devices_) {
    DeviceState& device = entry.second;
    const bool present = std::any_of(found.begin(), found.end(),
                                     [&entry](const DeviceState& other) {
                                       return other.port_name == entry.first;
                                     });
    if (!present && device.connection_state == ConnectionState::kConnected) {
      ClosePort(entry.first);
      device.connection_state = ConnectionState::kDisconnected;
      device.health = DeviceHealth::kUnknown;
      NotifySubscribers(DeviceEvent::kDisconnected, device);
    }
  }
  return utils::ErrorCode::kOk;
}

void DeviceManager::UpdateDevice(const DeviceState& found) {
  const auto existing = devices_.find(found.port_name);
  if (existing == devices_.end()) {
    devices_.emplace(found.port_name, found);
    NotifySubscribers(DeviceEvent::kAdded, found);
    return;
  }

  DeviceState& device = existing->second;
  if (device.device_name == found.device_name &&
      device.supported == found.supported &&
      device.connection_state == ConnectionState::kConnected) {
    return;
  }
  device.device_name = found.device_name;
  device.supported = found.supported;
  device.connection_state = ConnectionState::kConnected;
  NotifySubscribers(DeviceEvent::kUpdated, device);
}

std::vector<DeviceState> DeviceManager::ShowDevices() const {
  std::vector<DeviceState> result;
  result.reserve(devices_.size());

  for (const auto& entry : devices_) {
    result.push_back(entry.second);
  }
  return result;
}

utils::ErrorCode DeviceManager::GetDevice(DeviceRole role,
                                          DeviceState* device) const {
  if (device == nullptr) {
    return utils::ErrorCode::kInvalidArgument;
  }
  const auto match = std::find_if(
      devices_.begin(), devices_.end(),
      [role](const auto& entry) { return entry.second.role == role; });
  if (match == devices_.end()) {
    return utils::ErrorCode::kRoleNotAssigned;
  }
  *device = match->second;
  return utils::ErrorCode::kOk;
}

ConnectionState DeviceManager::GetConnectionState() const {
  const bool connected =
      std::any_of(devices_.begin(), devices_.end(), [](const auto& entry) {
        return entry.second.connection_state == ConnectionState::kConnected;
      });
  return connected ? ConnectionState::kConnected
                   : ConnectionState::kDisconnected;
}

utils::ErrorCode DeviceManager::SetRole(const std::string& port_name,
                                        DeviceRole role) {
  const auto target = devices_.find(port_name);
  if (target == devices_.end()) {
    return utils::ErrorCode::kPortNotFound;
  }
  if (target->second.role == role) {
    return utils::ErrorCode::kOk;
  }
  if (role != DeviceRole::kNone && !target->second.supported) {
    return utils::ErrorCode::kDeviceNotSupported;
  }

  if (role != DeviceRole::kNone) {
    for (auto& entry : devices_) {
      if (entry.first != port_name && entry.second.role == role) {
        entry.second.role = DeviceRole::kNone;
        NotifySubscribers(DeviceEvent::kUpdated, entry.second);
      }
    }
  }

  target->second.role = role;
  NotifySubscribers(DeviceEvent::kUpdated, target->second);
  return utils::ErrorCode::kOk;
}

utils::ErrorCode DeviceManager::SetBaudRate(const std::string& port_name,
                                            BaudRate rate) {
  const auto target = devices_.find(port_name);
  if (target == devices_.end()) {
    return utils::ErrorCode::kPortNotFound;
  }
  if (target->second.baud_rate != rate) {
    target->second.baud_rate = rate;
    NotifySubscribers(DeviceEvent::kUpdated, target->second);
  }
  return utils::ErrorCode::kOk;
}

utils::ErrorCode DeviceManager::SetHealth(const std::string& port_name,
                                          DeviceHealth health) {
  const auto target = devices_.find(port_name);
  if (target == devices_.end()) {
    return utils::ErrorCode::kPortNotFound;
  }
  if (target->second.health != health) {
    target->second.health = health;
    NotifySubscribers(DeviceEvent::kUpdated, target->second);
  }
  return utils::ErrorCode::kOk;
}

utils::ErrorCode DeviceManager::OpenPort(const std::string& port_name) {
  if (devices_.find(port_name) == devices_.end()) {
    return utils::ErrorCode::kPortNotFound;
  }
  if (open_ports_.find(port_name) != open_ports_.end()) {
    return utils::ErrorCode::kOk;
  }

  const std::string path = "\\\\.\\" + port_name;
  const HANDLE handle = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                                    0, nullptr, OPEN_EXISTING, 0, nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    return utils::ErrorCode::kPortOpenFailed;
  }

  const utils::ErrorCode result = ConfigurePort(handle, BaudRate::kBaud921600);
  if (!utils::IsOk(result)) {
    CloseHandle(handle);
    return result;
  }

  open_ports_[port_name] = handle;
  return SetBaudRate(port_name, BaudRate::kBaud921600);
}

utils::ErrorCode DeviceManager::ClosePort(const std::string& port_name) {
  const auto port = open_ports_.find(port_name);
  if (port == open_ports_.end()) {
    return utils::ErrorCode::kPortNotOpen;
  }
  CloseHandle(static_cast<HANDLE>(port->second));
  open_ports_.erase(port);
  return utils::ErrorCode::kOk;
}

utils::ErrorCode DeviceManager::SendFrame(DeviceRole role,
                                          const std::string& frame) {
  if (frame.empty()) {
    return utils::ErrorCode::kInvalidArgument;
  }
  void* port_handle = nullptr;
  const utils::ErrorCode result = FindOpenPort(role, &port_handle);
  if (!utils::IsOk(result)) {
    return result;
  }

  const HANDLE handle = static_cast<HANDLE>(port_handle);
  std::size_t total_written = 0;
  while (total_written < frame.size()) {
    DWORD written = 0;
    const auto remaining = static_cast<DWORD>(frame.size() - total_written);
    if (WriteFile(handle, frame.data() + total_written, remaining, &written,
                  nullptr) == 0 ||
        written == 0) {
      return utils::ErrorCode::kWriteFailed;
    }
    total_written += written;
  }
  return utils::ErrorCode::kOk;
}

utils::ErrorCode DeviceManager::ReceiveFrame(DeviceRole role,
                                             std::string* frame) {
  if (frame == nullptr) {
    return utils::ErrorCode::kInvalidArgument;
  }
  void* port_handle = nullptr;
  const utils::ErrorCode result = FindOpenPort(role, &port_handle);
  if (!utils::IsOk(result)) {
    return result;
  }

  const HANDLE handle = static_cast<HANDLE>(port_handle);
  frame->clear();
  std::array<char, kReadChunkSize> buffer{};
  for (;;) {
    DWORD read = 0;
    if (ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()),
                 &read, nullptr) == 0) {
      return utils::ErrorCode::kReadFailed;
    }
    if (read == 0) {
      break;
    }
    frame->append(buffer.data(), read);
    if (read < buffer.size()) {
      break;
    }
  }
  return frame->empty() ? utils::ErrorCode::kTimeout : utils::ErrorCode::kOk;
}

}  // namespace device_manager
}  // namespace core
}  // namespace stfuzz
