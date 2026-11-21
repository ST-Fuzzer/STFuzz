#include "adapters/TransportManager.hpp"

#include <QDateTime>

namespace stfuzz::adapters {

TransportManager::TransportManager(QIODevice* device, QObject* parent)
    : QObject(parent), deviceState(device) {
  if (deviceState) {
    connect(deviceState, &QIODevice::readyRead, this,
            &TransportManager::handleReadyRead);
  }
}

void TransportManager::setDevice(QIODevice* device) {
  if (deviceState) {
    disconnect(deviceState, nullptr, this, nullptr);
  }
  deviceState = device;
  if (deviceState) {
    connect(deviceState, &QIODevice::readyRead, this,
            &TransportManager::handleReadyRead);
  }
}

void TransportManager::transmit(const QByteArray& data, RequestId requestId) {
  Q_UNUSED(requestId);
  if (!deviceState || !deviceState->isOpen()) {
    TransportEvent event;
    event.eventType = -1;
    event.timestamp = QDateTime::currentDateTimeUtc();
    event.errorCode = 1;
    event.description = QStringLiteral("Device is not accessible or closed");
    emit transportEventOccurred(event);
    return;
  }

  const qint64 written = deviceState->write(data);
  TransportEvent event;
  event.timestamp = QDateTime::currentDateTimeUtc();
  event.bytesTransferred = written;
  if (written < 0) {
    event.errorCode = 2;
    event.description = deviceState->errorString();
  } else {
    event.errorCode = 0;
  }
  emit transportEventOccurred(event);
}

void TransportManager::onCommandReady(const QByteArray& data, RequestId id) {
  transmit(data, id);
}

void TransportManager::onConnectionStateChanged(ConnectionState state) {
  Q_UNUSED(state);
}

void TransportManager::handleReadyRead() {
  if (deviceState) {
    const QByteArray data = deviceState->readAll();
    if (!data.isEmpty()) {
      emit rawDataReceived(data);
    }
  }
}

}  // namespace stfuzz::adapters
