#include "core/device_manager.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "core/device_subscriber.hpp"
#include "utils/ErrorCode.hpp"

#ifdef _WIN32
// windows.h must come before setupapi.h, so keep clang-format from sorting
// them.
// clang-format off
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <setupapi.h>
// clang-format on
#endif

namespace stfuzz::core {

namespace {

constexpr std::uint16_t kStMicroelectronicsVendorId = 0x0483;

// One serial port as reported by the operating system.
struct PortInfo {
  std::string port_name;
  std::string description;
  std::string manufacturer;
  std::optional<std::uint16_t> vendor_id;
};

std::string ToLower(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return text;
}

bool ContainsIgnoreCase(const std::string& text, const std::string& needle) {
  return ToLower(text).find(ToLower(needle)) != std::string::npos;
}

std::string ResolveDeviceName(const PortInfo& port) {
  if (!port.description.empty()) {
    return port.description;
  }
  if (!port.manufacturer.empty()) {
    return port.manufacturer;
  }
  return port.port_name;
}

bool IsSupportedDevice(const PortInfo& port, const std::string& device_name) {
  // The USB vendor ID is more reliable than the description string.
  if (port.vendor_id == kStMicroelectronicsVendorId) {
    return true;
  }
  return ContainsIgnoreCase(device_name, "STM32") ||
         ContainsIgnoreCase(device_name, "STMicroelectronics");
}

bool IsValidBaudRate(BaudRate rate) {
  switch (rate) {
    case BaudRate::kBaud9600:
    case BaudRate::kBaud19200:
    case BaudRate::kBaud38400:
    case BaudRate::kBaud57600:
    case BaudRate::kBaud115200:
    case BaudRate::kBaud230400:
    case BaudRate::kBaud460800:
    case BaudRate::kBaud921600:
      return true;
  }
  return false;
}

#ifdef _WIN32

// Hardware IDs look like "USB\VID_0483&PID_5740&REV_0200".
std::optional<std::uint16_t> ParseVendorId(const std::string& hardware_id) {
  const std::string marker = "vid_";
  const std::string lowered = ToLower(hardware_id);
  const std::size_t position = lowered.find(marker);
  if (position == std::string::npos ||
      lowered.size() < position + marker.size() + 4) {
    return std::nullopt;
  }

  const std::string digits = lowered.substr(position + marker.size(), 4);
  char* end = nullptr;
  const unsigned long value = std::strtoul(digits.c_str(), &end, 16);
  if (end != digits.c_str() + digits.size()) {
    return std::nullopt;
  }
  return static_cast<std::uint16_t>(value);
}

std::string GetDeviceProperty(HDEVINFO device_info,
                              SP_DEVINFO_DATA* device_data, DWORD property) {
  std::vector<char> buffer(256, '\0');
  for (int attempt = 0; attempt < 2; ++attempt) {
    DWORD required_size = 0;
    if (SetupDiGetDeviceRegistryPropertyA(
            device_info, device_data, property, nullptr,
            reinterpret_cast<PBYTE>(buffer.data()),
            static_cast<DWORD>(buffer.size()), &required_size)) {
      buffer.back() = '\0';
      return std::string(buffer.data());
    }
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER ||
        required_size <= buffer.size()) {
      break;
    }
    buffer.assign(required_size, '\0');
  }
  return {};
}

// Reads the "COMx" name that the driver stored for this device.
std::string GetPortName(HDEVINFO device_info, SP_DEVINFO_DATA* device_data) {
  const HKEY key = SetupDiOpenDevRegKey(
      device_info, device_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
  if (key == INVALID_HANDLE_VALUE) {
    return {};
  }

  std::array<char, 256> name{};
  DWORD size = static_cast<DWORD>(name.size() - 1);
  DWORD type = 0;
  const LONG result =
      RegQueryValueExA(key, "PortName", nullptr, &type,
                       reinterpret_cast<LPBYTE>(name.data()), &size);
  RegCloseKey(key);

  if (result != ERROR_SUCCESS || type != REG_SZ) {
    return {};
  }
  return std::string(name.data());
}

// Fills `ports` with the COM ports that are currently present.
utils::ErrorCode ListSerialPorts(std::vector<PortInfo>* ports) {
  ports->clear();

  // Serial and parallel ports belong to the "Ports" device setup class.
  GUID ports_guid{};
  DWORD guid_count = 0;
  if (!SetupDiClassGuidsFromNameA("Ports", &ports_guid, 1, &guid_count) ||
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

  for (DWORD index = 0; SetupDiEnumDeviceInfo(device_info, index, &device_data);
       ++index) {
    std::string port_name = GetPortName(device_info, &device_data);
    if (port_name.rfind("COM", 0) != 0) {
      continue;  // Skip parallel (LPT) and unnamed ports.
    }

    PortInfo port;
    port.port_name = std::move(port_name);
    port.description =
        GetDeviceProperty(device_info, &device_data, SPDRP_DEVICEDESC);
    port.manufacturer = GetDeviceProperty(device_info, &device_data, SPDRP_MFG);
    port.vendor_id = ParseVendorId(
        GetDeviceProperty(device_info, &device_data, SPDRP_HARDWAREID));
    ports->push_back(std::move(port));
  }

  SetupDiDestroyDeviceInfoList(device_info);
  return utils::ErrorCode::kOk;
}

#else

// Port discovery is only implemented for Windows.
utils::ErrorCode ListSerialPorts(std::vector<PortInfo>* ports) {
  ports->clear();
  return utils::ErrorCode::kPortScanFailed;
}

#endif  // _WIN32

}  // namespace

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
  // Iterate over a copy so subscribers can subscribe or unsubscribe while being
  // notified. A subscriber removed during this pass is skipped.
  const std::vector<IDeviceSubscriber*> snapshot = subscribers_;
  for (IDeviceSubscriber* subscriber : snapshot) {
    if (std::find(subscribers_.begin(), subscribers_.end(), subscriber) !=
        subscribers_.end()) {
      subscriber->Update(event, device);
    }
  }
}

utils::ErrorCode DeviceManager::ScanPorts() {
  // Query the system once per scan and reuse the result.
  std::vector<PortInfo> ports;
  const utils::ErrorCode result = ListSerialPorts(&ports);
  if (!utils::IsOk(result)) {
    return result;  // Keep the known devices: we cannot tell what changed.
  }

  std::set<std::string> detected_ports;
  for (const PortInfo& port : ports) {
    detected_ports.insert(port.port_name);

    const std::string device_name = ResolveDeviceName(port);
    UpdateDevice(port.port_name, device_name,
                 IsSupportedDevice(port, device_name));
  }

  // Mark devices that are no longer present as disconnected.
  for (auto& [port_name, device] : devices_) {
    if (device.connection_state == ConnectionState::kConnected &&
        detected_ports.count(port_name) == 0) {
      device.connection_state = ConnectionState::kDisconnected;
      device.health = DeviceHealth::kUnknown;
      NotifySubscribers(DeviceEvent::kDisconnected, device);
    }
  }
  return utils::ErrorCode::kOk;
}

void DeviceManager::UpdateDevice(const std::string& port_name,
                                 const std::string& device_name,
                                 bool supported) {
  const auto existing = devices_.find(port_name);
  const bool is_new_device = (existing == devices_.end());

  DeviceState device;
  if (!is_new_device) {
    device = existing->second;
  }
  const DeviceState previous = device;

  device.port_name = port_name;
  device.device_name = device_name;
  device.supported = supported;
  device.connection_state = ConnectionState::kConnected;

  const bool changed = is_new_device ||
                       device.device_name != previous.device_name ||
                       device.supported != previous.supported ||
                       device.connection_state != previous.connection_state;
  if (!changed) {
    return;  // Nothing new: avoid notifying on every scan.
  }

  devices_[port_name] = device;
  NotifySubscribers(is_new_device ? DeviceEvent::kAdded : DeviceEvent::kUpdated,
                    device);
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
  for (const auto& entry : devices_) {
    if (entry.second.role == role) {
      *device = entry.second;
      return utils::ErrorCode::kOk;
    }
  }
  return utils::ErrorCode::kRoleNotAssigned;
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

  // A role can only be held by one device at a time.
  if (role != DeviceRole::kNone) {
    for (auto& [name, device] : devices_) {
      if (name != port_name && device.role == role) {
        device.role = DeviceRole::kNone;
        NotifySubscribers(DeviceEvent::kUpdated, device);
      }
    }
  }

  target->second.role = role;
  NotifySubscribers(DeviceEvent::kUpdated, target->second);
  return utils::ErrorCode::kOk;
}

utils::ErrorCode DeviceManager::SetBaudRate(const std::string& port_name,
                                            BaudRate rate) {
  if (!IsValidBaudRate(rate)) {
    return utils::ErrorCode::kInvalidBaudRate;
  }

  const auto target = devices_.find(port_name);
  if (target == devices_.end()) {
    return utils::ErrorCode::kPortNotFound;
  }
  if (target->second.baud_rate == rate) {
    return utils::ErrorCode::kOk;
  }

  target->second.baud_rate = rate;
  NotifySubscribers(DeviceEvent::kUpdated, target->second);
  return utils::ErrorCode::kOk;
}

utils::ErrorCode DeviceManager::SetHealth(const std::string& port_name,
                                          DeviceHealth health) {
  const auto target = devices_.find(port_name);
  if (target == devices_.end()) {
    return utils::ErrorCode::kPortNotFound;
  }
  if (target->second.health == health) {
    return utils::ErrorCode::kOk;
  }

  target->second.health = health;
  NotifySubscribers(DeviceEvent::kUpdated, target->second);
  return utils::ErrorCode::kOk;
}

ConnectionState DeviceManager::GetConnectionState() const {
  for (const auto& entry : devices_) {
    if (entry.second.connection_state == ConnectionState::kConnected) {
      return ConnectionState::kConnected;
    }
  }
  return ConnectionState::kDisconnected;
}

}  // namespace stfuzz::core
