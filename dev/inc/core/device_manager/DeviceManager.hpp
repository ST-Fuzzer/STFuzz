#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "core/device_manager/DeviceSubscriber.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace device_manager {

enum class DeviceRole : std::uint8_t { kNone, kDut, kInjector, kSniffer };

enum class DeviceHealth : std::uint8_t { kUnknown, kHealthy, kCrashed };

enum class BaudRate {
  kBaud9600 = 9600,
  kBaud19200 = 19200,
  kBaud38400 = 38400,
  kBaud57600 = 57600,
  kBaud115200 = 115200,
  kBaud230400 = 230400,
  kBaud460800 = 460800,
  kBaud921600 = 921600
};

enum class ConnectionState : std::uint8_t { kDisconnected, kConnected };

struct DeviceState {
  std::string port_name;
  std::string device_name;

  DeviceRole role = DeviceRole::kNone;
  DeviceHealth health = DeviceHealth::kUnknown;
  BaudRate baud_rate = BaudRate::kBaud115200;
  ConnectionState connection_state = ConnectionState::kDisconnected;

  bool supported = false;
};

class DeviceManager {
 public:
  DeviceManager() = default;
  ~DeviceManager();

  DeviceManager(const DeviceManager&) = delete;
  DeviceManager& operator=(const DeviceManager&) = delete;

  utils::ErrorCode Subscribe(IDeviceSubscriber* subscriber);
  void Unsubscribe(IDeviceSubscriber* subscriber);

  utils::ErrorCode ScanPorts();

  std::vector<DeviceState> ShowDevices() const;
  utils::ErrorCode GetDevice(DeviceRole role, DeviceState* device) const;
  ConnectionState GetConnectionState() const;

  utils::ErrorCode SetRole(const std::string& port_name, DeviceRole role);
  utils::ErrorCode SetBaudRate(const std::string& port_name, BaudRate rate);
  utils::ErrorCode SetHealth(const std::string& port_name, DeviceHealth health);

  utils::ErrorCode OpenPort(const std::string& port_name);
  utils::ErrorCode ClosePort(const std::string& port_name);
  utils::ErrorCode SendFrame(DeviceRole role, const std::string& frame);
  utils::ErrorCode ReceiveFrame(DeviceRole role, std::string* frame);

 private:
  void UpdateDevice(const DeviceState& found);
  void NotifySubscribers(DeviceEvent event, const DeviceState& device);
  utils::ErrorCode FindOpenPort(DeviceRole role, void** handle) const;

  std::vector<IDeviceSubscriber*> subscribers_;
  std::map<std::string, DeviceState> devices_;
  std::map<std::string, void*> open_ports_;
};

}  // namespace device_manager
}  // namespace core
}  // namespace stfuzz