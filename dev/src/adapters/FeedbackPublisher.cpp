#include "adapters/FeedbackPublisher.hpp"

#include <QtCore/qtmetamacros.h>

#include <QDateTime>
#include <QObject>
#include <QVariant>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

FeedbackPublisher::FeedbackPublisher(QObject* parent) : QObject(parent) {}

void FeedbackPublisher::setActiveTestcaseId(TestcaseId id) {
  activeTestcaseId = id;
}

void FeedbackPublisher::onCommandResultReady(const CommandResult& result) {
  const DUTFeedback feedback =
      createFeedback(FeedbackSource::CommandFeedbackManager,
                     FeedbackType::COMMAND_RESULT, QVariant::fromValue(result));
  publishFeedback(feedback);
}

void FeedbackPublisher::onTelemetryRecordReady(const TelemetryRecord& record) {
  const DUTFeedback feedback = createFeedback(
      FeedbackSource::TelemetryProcessor, FeedbackType::TELEMETRY_RECORD,
      QVariant::fromValue(record));
  publishFeedback(feedback);
}

void FeedbackPublisher::onTelemetryEventReady(const TelemetryEvent& event) {
  const DUTFeedback feedback =
      createFeedback(FeedbackSource::TelemetryProcessor,
                     FeedbackType::TELEMETRY_EVENT, QVariant::fromValue(event));
  publishFeedback(feedback);
}

void FeedbackPublisher::onHealthEventDetected(const HealthEvent& event) {
  const DUTFeedback feedback =
      createFeedback(FeedbackSource::HealthMonitor, FeedbackType::HEALTH_EVENT,
                     QVariant::fromValue(event));
  publishFeedback(feedback);
}

void FeedbackPublisher::onTransportErrorReported(const TransportEvent& event) {
  const DUTFeedback feedback =
      createFeedback(FeedbackSource::TransportFeedbackManager,
                     FeedbackType::TRANSPORT_ERROR, QVariant::fromValue(event));
  publishFeedback(feedback);
}

void FeedbackPublisher::onParsingErrorDetected(const ParsingError& error) {
  const DUTFeedback feedback =
      createFeedback(FeedbackSource::TelemetryProcessor,
                     FeedbackType::PARSING_ERROR, QVariant::fromValue(error));
  publishFeedback(feedback);
}

void FeedbackPublisher::onConnectionStateChanged(ConnectionState state) {
  const DUTFeedback feedback = createFeedback(
      FeedbackSource::DeviceManager, FeedbackType::CONNECTION_STATE,
      QVariant::fromValue(static_cast<int>(state)));
  publishFeedback(feedback);
}

DUTFeedback FeedbackPublisher::createFeedback(FeedbackSource source,
                                              FeedbackType type,
                                              const QVariant& payload) {
  DUTFeedback feedback;
  feedback.testcaseId = activeTestcaseId;
  feedback.sequenceNumber = nextSequenceNumber++;
  feedback.timestamp = QDateTime::currentDateTimeUtc();
  feedback.source = source;
  feedback.type = type;
  feedback.payload = payload;
  return feedback;
}

void FeedbackPublisher::publishFeedback(const DUTFeedback& feedback) {
  Q_EMIT feedbackReady(feedback);
}

}  // namespace stfuzz::adapters
