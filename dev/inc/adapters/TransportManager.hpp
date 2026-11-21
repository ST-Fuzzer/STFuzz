#pragma once

#include <QByteArray>
#include <QIODevice>
#include <QObject>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

class TransportManager : public QObject {
  Q_OBJECT

 public:
  explicit TransportManager(QIODevice* device = nullptr,
                            QObject* parent = nullptr);
  ~TransportManager() override = default;

  void setDevice(QIODevice* device);
  void transmit(const QByteArray& data, RequestId requestId);

 public slots:
  void onCommandReady(const QByteArray& data, RequestId id);
  void onConnectionStateChanged(ConnectionState state);

 signals:
  void rawDataReceived(const QByteArray& data);
  void transportEventOccurred(const TransportEvent& event);

 private slots:
  void handleReadyRead();

 private:
  QIODevice* deviceState{nullptr};
};

}  // namespace stfuzz::adapters
