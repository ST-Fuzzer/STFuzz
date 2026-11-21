#pragma once

#include <QDateTime>
#include <QObject>
#include <QTimer>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

class HealthMonitor : public QObject {
  Q_OBJECT

 public:
  explicit HealthMonitor(QObject* parent = nullptr);
  ~HealthMonitor() override = default;

  void configureHeartbeatTimeout(int timeout_ms);
  DUTHealthState getHealthState() const { return currentState; }

 public slots:
  void onTelemetryEvent(const TelemetryEvent& event);
  void onCommandTimeoutDetected(const CommandTimeoutEvent& event);
  void onConnectionStateChanged(ConnectionState state);
  void onTransportErrorReported(const TransportEvent& event);

 signals:
  void healthEventDetected(const HealthEvent& event);

 private slots:
  void onHeartbeatTimeout();

 private:
  void evaluateHealth();
  void updateHealthState(DUTHealthState state);

  DUTHealthState currentState{DUTHealthState::Unknown};
  ConnectionState connectionState{ConnectionState::Disconnected};
  QDateTime lastHeartbeatAt;
  int heartbeatTimeoutMs{5000};
  QTimer* heartbeatTimer{nullptr};
};

}  // namespace stfuzz::adapters
