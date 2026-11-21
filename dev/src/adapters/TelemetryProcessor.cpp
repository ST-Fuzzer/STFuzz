#include "adapters/TelemetryProcessor.hpp"

#include <QtCore/qtmetamacros.h>

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>

#include "adapters/ITelemetryDecoder.hpp"
#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

TelemetryProcessor::TelemetryProcessor(ITelemetryDecoder* decoder,
                                       QObject* parent)
    : QObject(parent), decoder(decoder) {}

void TelemetryProcessor::setDecoder(ITelemetryDecoder* decoder_ptr) {
  decoder = decoder_ptr;
}

void TelemetryProcessor::clearReceiveBuffer() { receiveBuffer.clear(); }

void TelemetryProcessor::onRawDataReceived(const QByteArray& data) {
  processReceivedData(data);
}

void TelemetryProcessor::onConnectionStateChanged(ConnectionState state) {
  if (state == ConnectionState::Disconnected) {
    clearReceiveBuffer();
  }
}

void TelemetryProcessor::processReceivedData(const QByteArray& data) {
  receiveBuffer.append(data);

  if (receiveBuffer.size() > maxBufferSize) {
    ParsingError err;
    err.timestamp = QDateTime::currentDateTimeUtc();
    err.description =
        QStringLiteral("Receive buffer boundary exceeded max size");
    err.rawFrame = receiveBuffer;

    Q_EMIT parsingErrorDetected(err);
    clearReceiveBuffer();
    return;
  }

  const QList<QByteArray> frames = extractFrames();
  for (const auto& frame : frames) {
    if (decoder != nullptr) {
      const DecodedMessage msg = decoder->decodeFrame(frame);
      dispatchMessage(msg);
    }
  }
}

QList<QByteArray> TelemetryProcessor::extractFrames() {
  QList<QByteArray> frames;
  if (!receiveBuffer.isEmpty()) {
    frames.append(receiveBuffer);
    receiveBuffer.clear();
  }
  return frames;
}

void TelemetryProcessor::dispatchMessage(const DecodedMessage& message) {
  if (message.messageType == 0) {
    CommandResponse resp;
    resp.correlationId = message.sequenceNumber;
    resp.responseType = message.messageType;
    resp.payload = message.payload;
    resp.receivedAt = message.receivedAt;
    Q_EMIT commandResponseDecoded(resp);
  } else if (message.messageType == 1) {
    TelemetryRecord rec;
    rec.recordType = message.messageType;
    rec.sequenceNumber = message.sequenceNumber;
    rec.timestamp = message.receivedAt;
    Q_EMIT telemetryRecordReady(rec);
  } else if (message.messageType == 2) {
    TelemetryEvent ev;
    ev.eventType = message.messageType;
    ev.timestamp = message.receivedAt;
    Q_EMIT telemetryEventReady(ev);
  }
}

}  // namespace stfuzz::adapters
