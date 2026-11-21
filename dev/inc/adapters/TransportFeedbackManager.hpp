#pragma once

#include <QObject>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

class TransportFeedbackManager : public QObject {
  Q_OBJECT

 public:
  explicit TransportFeedbackManager(QObject* parent = nullptr);
  ~TransportFeedbackManager() override = default;

  TransportStatus getStatus() const { return currentStatus; }
  TransportEvent getLastError() const { return lastError; }

 public slots:
  void onTransportEvent(const TransportEvent& event);

 signals:
  void commandTransmissionResult(const TransmissionResult& result);
  void transportErrorReported(const TransportEvent& event);
  void transportStatusChanged(TransportStatus status);

 private:
  void updateStatus(const TransportEvent& event);
  static TransmissionResult createTransmissionResult(
      const TransportEvent& event);

  TransportStatus currentStatus{TransportStatus::Disconnected};
  TransportEvent lastError;
};

}  // namespace stfuzz::adapters
