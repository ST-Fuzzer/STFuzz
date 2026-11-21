#pragma once

#include <QObject>
#include <QVariant>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

class FeedbackPublisher : public QObject {
  Q_OBJECT

 public:
  explicit FeedbackPublisher(QObject* parent = nullptr);
  ~FeedbackPublisher() override = default;

  void setActiveTestcaseId(TestcaseId id);
  TestcaseId getActiveTestcaseId() const { return activeTestcaseId; }

 public slots:
  void onCommandResultReady(const CommandResult& result);
  void onTelemetryRecordReady(const TelemetryRecord& record);
  void onTelemetryEventReady(const TelemetryEvent& event);
  void onHealthEventDetected(const HealthEvent& event);
  void onTransportErrorReported(const TransportEvent& event);
  void onParsingErrorDetected(const ParsingError& error);
  void onConnectionStateChanged(ConnectionState state);

 signals:
  void feedbackReady(const DUTFeedback& feedback);

 private:
  DUTFeedback createFeedback(FeedbackSource source, FeedbackType type,
                             const QVariant& payload);
  void publishFeedback(const DUTFeedback& feedback);

  TestcaseId activeTestcaseId{0};
  quint64 nextSequenceNumber{1};
};

}  // namespace stfuzz::adapters
