#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>

class QSerialPortInfo;

namespace stfuzz::core {

enum class DeviceRole { kNone, kDut, kInjector, kSniffer };

enum class DeviceHealth { kUnknown, kHealthy, kDegraded, kUnhealthy };

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

enum class ConnectionState { kDisconnected, kConnected };

struct DeviceState {
  QString port_name;
  QString device_name;

  DeviceRole role = DeviceRole::kNone;
  DeviceHealth health = DeviceHealth::kUnknown;
  BaudRate baud_rate = BaudRate::kBaud115200;
  ConnectionState connection_state = ConnectionState::kDisconnected;

  bool supported = false;
};

// Discovers serial devices, tracks their state, and notifies listeners when
// devices are added, changed or removed.
//
// Devices that disappear are kept in the list with
// ConnectionState::kDisconnected so their role and baud rate survive a
// reconnect.
class DeviceManager : public QObject {
  Q_OBJECT

 public:
  explicit DeviceManager(QObject* parent = nullptr);

  void StartMonitoring();
  void StopMonitoring();
  bool IsMonitoring() const;

  // Scans the system once. Called automatically while monitoring.
  void ScanPorts();

  QList<DeviceState> ShowDevices() const;

  // Returns the device holding `role`, or a default DeviceState (empty
  // port_name) if no device has it.
  DeviceState GetDevice(DeviceRole role) const;

  // A role belongs to at most one device; assigning it moves it.
  void SetRole(const QString& port_name, DeviceRole role);
  void SetBaudRate(const QString& port_name, BaudRate rate);

  // Connected if at least one device is connected.
  ConnectionState GetConnectionState() const;

 signals:
  void DeviceAdded(const stfuzz::core::DeviceState& device);
  void DeviceUpdated(const stfuzz::core::DeviceState& device);
  void DeviceDisconnected(const stfuzz::core::DeviceState& device);

 public slots:
  void OnDeviceHealthChanged(const QString& port_name, DeviceHealth health);

 private:
  void UpdateDevice(const QSerialPortInfo& port_info);
  static bool IsSupportedDevice(const QSerialPortInfo& port_info,
                                const QString& device_name);

  QHash<QString, DeviceState> devices_;
  QTimer scan_timer_{this};
};

}  // namespace stfuzz::core

Q_DECLARE_METATYPE(stfuzz::core::DeviceRole)
Q_DECLARE_METATYPE(stfuzz::core::DeviceHealth)
Q_DECLARE_METATYPE(stfuzz::core::BaudRate)
Q_DECLARE_METATYPE(stfuzz::core::ConnectionState)
Q_DECLARE_METATYPE(stfuzz::core::DeviceState)
