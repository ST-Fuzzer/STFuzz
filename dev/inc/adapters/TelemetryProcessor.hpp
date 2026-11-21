#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>

#include "adapters/ITelemetryDecoder.hpp"
#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

class TelemetryProcessor : public QObject {
  Q_OBJECT

 public:
  explicit TelemetryProcessor(ITelemetryDecoder* decoder = nullptr,
                              QObject* parent = nullptr);
  ~TelemetryProcessor() override = default;

  void setDecoder(ITelemetryDecoder* decoder);
  void clearReceiveBuffer();

 public slots:
  void onRawDataReceived(const QByteArray& data);
  void onConnectionStateChanged(ConnectionState state);

 signals:
  void parsingErrorDetected(const ParsingError& error);
  void commandResponseDecoded(const CommandResponse& response);
  void telemetryRecordReady(const TelemetryRecord& record);
  void telemetryEventReady(const TelemetryEvent& event);

 private:
  void processReceivedData(const QByteArray& data);
  QList<QByteArray> extractFrames();
  void dispatchMessage(const DecodedMessage& message);

  QByteArray receiveBuffer;
  ITelemetryDecoder* decoder{nullptr};
  qsizetype maxBufferSize{65536};
};

}  // namespace stfuzz::adapters
