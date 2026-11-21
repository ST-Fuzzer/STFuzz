#include "adapters/TransportManager.hpp"

#include <QtCore/qtmetamacros.h>

#include <QByteArray>
#include <QDateTime>
#include <QIODevice>
#include <QObject>
#include <QString>
#include <QtTypes>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

TransportManager::TransportManager(QIODevice* device, QObject* parent)
    : QObject(parent), deviceState(device) {
  if (deviceState != nullptr) {
    connect(deviceState, &QIODevice::readyRead, this,
            &TransportManager::handleReadyRead);
  }
}

void TransportManager::setDevice(QIODevice* device) {
  if (deviceState != nullptr) {
    disconnect(deviceState, nullptr, this, nullptr);
  }
  deviceState = device;
  if (deviceState != nullptr) {
    connect(deviceState, &QIODevice::readyRead, this,
            &TransportManager::handleReadyRead);
  }
}

void TransportManager::transmit(const QByteArray& data,
                                RequestId /*request_id*/) {
  if (deviceState == nullptr || !deviceState->isOpen()) {
    TransportEvent event;
    event.eventType = -1;
    event.timestamp = QDateTime::currentDateTimeUtc();
    event.errorCode = 1;
    event.description = QStringLiteral("Device is not accessible or closed");
    Q_EMIT transportEventOccurred(event);
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
  Q_EMIT transportEventOccurred(event);
}

void TransportManager::onCommandReady(const QByteArray& data, RequestId id) {
  transmit(data, id);
}

void TransportManager::onConnectionStateChanged(ConnectionState /*state*/) {}

void TransportManager::handleReadyRead() {
  if (deviceState != nullptr) {
    const QByteArray data = deviceState->readAll();
    if (!data.isEmpty()) {
      Q_EMIT rawDataReceived(data);
    }
  }
}

}  // namespace stfuzz::adapters
