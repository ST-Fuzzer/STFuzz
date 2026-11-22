#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "core/device_subscriber.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz::core {

enum class DeviceRole : std::uint8_t { kNone, kDut, kInjector, kSniffer };

enum class DeviceHealth : std::uint8_t {
  kUnknown,
  kHealthy,
  kDegraded,
  kUnhealthy
};

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

// Discovers serial (COM) ports with the Windows SetupAPI, tracks their state,
// and notifies its subscribers (see IDeviceSubscriber) when devices are added,
// changed or disconnected. It is the publisher of the observer pattern.
//
// Operations that can fail return a utils::ErrorCode (utils::ErrorCode::kOk on
// success). Port discovery is only implemented for Windows; on other platforms
// ScanPorts() returns utils::ErrorCode::kPortScanFailed.
//
// Devices that disappear are kept in the list with
// ConnectionState::kDisconnected so their role and baud rate survive a
// reconnect.
//
// Threading: not thread-safe. Subscribers are called synchronously on the
// thread that triggered the change. A subscriber may subscribe or unsubscribe
// from inside Update(), but must not call ScanPorts().
class DeviceManager {
 public:
  // Subscribers are not owned. A subscriber must call Unsubscribe() before it
  // is destroyed.
  //   kInvalidArgument: `subscriber` is null.
  // Subscribing an already subscribed subscriber does nothing and returns kOk.
  [[nodiscard]] utils::ErrorCode Subscribe(IDeviceSubscriber* subscriber);
  void Unsubscribe(IDeviceSubscriber* subscriber);

  // Scans the system once. The caller decides how often (timer, thread, ...).
  //   kPortScanFailed: the system could not list the ports. The device list is
  //                    left unchanged.
  [[nodiscard]] utils::ErrorCode ScanPorts();

  [[nodiscard]] std::vector<DeviceState> ShowDevices() const;

  // Copies the device that holds `role` into `*device`.
  //   kInvalidArgument: `device` is null.
  //   kRoleNotAssigned: no device holds `role`.
  [[nodiscard]] utils::ErrorCode GetDevice(DeviceRole role,
                                           DeviceState* device) const;

  // A role belongs to at most one device; assigning it moves it. Only supported
  // devices can take a role; DeviceRole::kNone is always allowed, so a role can
  // still be cleared.
  //   kPortNotFound:       no device with this port name.
  //   kDeviceNotSupported: the device is not supported and `role` is not kNone.
  [[nodiscard]] utils::ErrorCode SetRole(const std::string& port_name,
                                         DeviceRole role);

  //   kInvalidBaudRate: `rate` is not one of the BaudRate values.
  //   kPortNotFound:    no device with this port name.
  [[nodiscard]] utils::ErrorCode SetBaudRate(const std::string& port_name,
                                             BaudRate rate);

  //   kPortNotFound: no device with this port name.
  [[nodiscard]] utils::ErrorCode SetHealth(const std::string& port_name,
                                           DeviceHealth health);

  // Connected if at least one device is connected.
  [[nodiscard]] ConnectionState GetConnectionState() const;

 private:
  // Calls Update() on every subscriber.
  void NotifySubscribers(DeviceEvent event, const DeviceState& device);

  void UpdateDevice(const std::string& port_name,
                    const std::string& device_name, bool supported);

  std::vector<IDeviceSubscriber*> subscribers_;
  std::map<std::string, DeviceState> devices_;
};

}  // namespace stfuzz::core
