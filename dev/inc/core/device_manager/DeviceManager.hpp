#pragma once

#include <map>
#include <string>
#include <vector>

#include "core/device_manager/DeviceSubscriber.hpp"
#include "core/device_manager/DeviceTypes.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace device_manager {

class DeviceManager {
 public:
  [[nodiscard]] stfuzz::utils::ErrorCode ScanPorts();

  [[nodiscard]] std::vector<DeviceState> ShowDevices() const;
  [[nodiscard]] stfuzz::utils::ErrorCode GetDevice(DeviceRole role,
                                                   DeviceState* device) const;
  [[nodiscard]] ConnectionState GetConnectionState() const;

  [[nodiscard]] stfuzz::utils::ErrorCode SetRole(const std::string& port_name,
                                                 DeviceRole role);
  [[nodiscard]] stfuzz::utils::ErrorCode SetBaudRate(
      const std::string& port_name, BaudRate rate);
  [[nodiscard]] stfuzz::utils::ErrorCode SetHealth(const std::string& port_name,
                                                   DeviceHealth health);

  [[nodiscard]] stfuzz::utils::ErrorCode Subscribe(
      IDeviceSubscriber* subscriber);
  void Unsubscribe(IDeviceSubscriber* subscriber);

 private:
  void UpdateDevice(const DeviceState& found);
  void NotifySubscribers(DeviceEvent event, const DeviceState& device);

  std::vector<IDeviceSubscriber*> subscribers_;
  std::map<std::string, DeviceState> devices_;
};

}  // namespace device_manager
}  // namespace core
}  // namespace stfuzz
