#include "core/device_manager/DeviceManager.hpp"

#include <setupapi.h>
#include <windows.h>

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
  const std::vector<IDeviceSubscriber*> snapshot = subscribers_;
  for (IDeviceSubscriber* subscriber : snapshot) {
    if (std::find(subscribers_.begin(), subscribers_.end(), subscriber) !=
        subscribers_.end()) {
      subscriber->Update(event, device);
    }
  }
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

}  // namespace device_manager
}  // namespace core
}  // namespace stfuzz
