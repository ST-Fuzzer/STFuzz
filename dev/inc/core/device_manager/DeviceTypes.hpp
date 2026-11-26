#pragma once

#include <cstdint>
#include <string>

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
  kBaud921600 = 921600,
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

}  // namespace device_manager
}  // namespace core
}  // namespace stfuzz
