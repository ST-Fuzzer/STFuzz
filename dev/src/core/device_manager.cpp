#include "core/device_manager.hpp"

#include <QtCore/qnamespace.h>
#include <QtCore/qstringliteral.h>
#include <QtCore/qtypes.h>

#include <QList>
#include <QMetaType>
#include <QObject>
#include <QSerialPortInfo>
#include <QSet>
#include <QString>

namespace stfuzz::core {

namespace {

constexpr int kScanIntervalMs = 1000;
constexpr quint16 kStMicroelectronicsVendorId = 0x0483;

QString ResolveDeviceName(const QSerialPortInfo& port_info) {
  if (!port_info.description().isEmpty()) {
    return port_info.description();
  }
  if (!port_info.manufacturer().isEmpty()) {
    return port_info.manufacturer();
  }
  return port_info.portName();
}

}  // namespace

DeviceManager::DeviceManager(QObject* parent) : QObject(parent) {
  qRegisterMetaType<DeviceRole>();
  qRegisterMetaType<DeviceHealth>();
  qRegisterMetaType<BaudRate>();
  qRegisterMetaType<ConnectionState>();
  qRegisterMetaType<DeviceState>();

  scan_timer_.setInterval(kScanIntervalMs);
  connect(&scan_timer_, &QTimer::timeout, this, &DeviceManager::ScanPorts);
}

void DeviceManager::StartMonitoring() {
  if (scan_timer_.isActive()) {
    return;
  }
  ScanPorts();
  scan_timer_.start();
}

void DeviceManager::StopMonitoring() { scan_timer_.stop(); }

bool DeviceManager::IsMonitoring() const { return scan_timer_.isActive(); }

void DeviceManager::ScanPorts() {
  // Query the system once per scan and reuse the result.
  const QList<QSerialPortInfo> available_ports =
      QSerialPortInfo::availablePorts();

  QSet<QString> detected_ports;
  detected_ports.reserve(available_ports.size());

  for (const QSerialPortInfo& port_info : available_ports) {
    detected_ports.insert(port_info.portName());
    UpdateDevice(port_info);
  }

  // Mark devices that are no longer present as disconnected.
  for (auto it = devices_.begin(); it != devices_.end(); ++it) {
    if (it->connection_state == ConnectionState::kConnected &&
        !detected_ports.contains(it.key())) {
      it->connection_state = ConnectionState::kDisconnected;
      it->health = DeviceHealth::kUnknown;

      DeviceDisconnected(it.value());
    }
  }
}

void DeviceManager::UpdateDevice(const QSerialPortInfo& port_info) {
  const QString port_name = port_info.portName();

  const auto existing = devices_.constFind(port_name);
  const bool is_new_device = (existing == devices_.constEnd());

  DeviceState device = is_new_device ? DeviceState{} : existing.value();
  const DeviceState previous = device;

  device.port_name = port_name;
  device.device_name = ResolveDeviceName(port_info);
  device.supported = IsSupportedDevice(port_info, device.device_name);
  device.connection_state = ConnectionState::kConnected;

  const bool changed = is_new_device ||
                       device.device_name != previous.device_name ||
                       device.supported != previous.supported ||
                       device.connection_state != previous.connection_state;

  if (!changed) {
    return;  // Nothing new: avoid emitting a signal every scan.
  }

  devices_.insert(port_name, device);

  if (is_new_device) {
    DeviceAdded(device);
  } else {
    DeviceUpdated(device);
  }
}

bool DeviceManager::IsSupportedDevice(const QSerialPortInfo& port_info,
                                      const QString& device_name) {
  // The USB vendor ID is more reliable than the description string.
  if (port_info.hasVendorIdentifier() &&
      port_info.vendorIdentifier() == kStMicroelectronicsVendorId) {
    return true;
  }

  return device_name.contains(QStringLiteral("STM32"), Qt::CaseInsensitive) ||
         device_name.contains(QStringLiteral("STMicroelectronics"),
                              Qt::CaseInsensitive);
}

QList<DeviceState> DeviceManager::ShowDevices() const {
  return devices_.values();
}

DeviceState DeviceManager::GetDevice(DeviceRole role) const {
  for (const DeviceState& device : devices_) {
    if (device.role == role) {
      return device;
    }
  }
  return {};
}

void DeviceManager::SetRole(const QString& port_name, DeviceRole role) {
  const auto target = devices_.find(port_name);
  if (target == devices_.end() || target->role == role) {
    return;
  }

  // A role can only be held by one device at a time.
  if (role != DeviceRole::kNone) {
    for (auto it = devices_.begin(); it != devices_.end(); ++it) {
      if (it.key() != port_name && it->role == role) {
        it->role = DeviceRole::kNone;
        DeviceUpdated(it.value());
      }
    }
  }

  target->role = role;
  DeviceUpdated(target.value());
}

void DeviceManager::SetBaudRate(const QString& port_name, BaudRate rate) {
  const auto it = devices_.find(port_name);
  if (it == devices_.end() || it->baud_rate == rate) {
    return;
  }

  it->baud_rate = rate;
  DeviceUpdated(it.value());
}

ConnectionState DeviceManager::GetConnectionState() const {
  for (const DeviceState& device : devices_) {
    if (device.connection_state == ConnectionState::kConnected) {
      return ConnectionState::kConnected;
    }
  }
  return ConnectionState::kDisconnected;
}

void DeviceManager::OnDeviceHealthChanged(const QString& port_name,
                                          DeviceHealth health) {
  const auto it = devices_.find(port_name);
  if (it == devices_.end() || it->health == health) {
    return;
  }

  it->health = health;
  DeviceUpdated(it.value());
}

}  // namespace stfuzz::core
