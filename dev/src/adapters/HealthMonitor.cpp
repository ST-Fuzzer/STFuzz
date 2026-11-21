#include "adapters/HealthMonitor.hpp"

#include <QtCore/qtmetamacros.h>

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

HealthMonitor::HealthMonitor(QObject* parent) : QObject(parent) {
  heartbeatTimer = new QTimer(this);
  connect(heartbeatTimer, &QTimer::timeout, this,
          &HealthMonitor::onHeartbeatTimeout);
}

void HealthMonitor::configureHeartbeatTimeout(int timeout_ms) {
  heartbeatTimeoutMs = timeout_ms;
  if (heartbeatTimer->isActive()) {
    heartbeatTimer->start(heartbeatTimeoutMs);
  }
}

void HealthMonitor::onTelemetryEvent(const TelemetryEvent& /*event*/) {
  lastHeartbeatAt = QDateTime::currentDateTimeUtc();
  evaluateHealth();
}

void HealthMonitor::onCommandTimeoutDetected(
    const CommandTimeoutEvent& /*event*/) {
  updateHealthState(DUTHealthState::SuspectedUnresponsive);
}

void HealthMonitor::onConnectionStateChanged(ConnectionState state) {
  connectionState = state;
  if (connectionState == ConnectionState::Connected) {
    heartbeatTimer->start(heartbeatTimeoutMs);
    updateHealthState(DUTHealthState::Responsive);
  } else {
    heartbeatTimer->stop();
    updateHealthState(DUTHealthState::Unknown);
  }
}

void HealthMonitor::onTransportErrorReported(const TransportEvent& /*event*/) {
  updateHealthState(DUTHealthState::SuspectedUnresponsive);
}

void HealthMonitor::onHeartbeatTimeout() {
  updateHealthState(DUTHealthState::SuspectedUnresponsive);
}

void HealthMonitor::evaluateHealth() {
  updateHealthState(DUTHealthState::Responsive);
}

void HealthMonitor::updateHealthState(DUTHealthState state) {
  if (currentState != state) {
    currentState = state;
    HealthEvent ev;
    ev.healthState = currentState;
    ev.timestamp = QDateTime::currentDateTimeUtc();
    ev.description = QStringLiteral("DUT health state changed");
    Q_EMIT healthEventDetected(ev);
  }
}

}  // namespace stfuzz::adapters
