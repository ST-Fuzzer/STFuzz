#include "adapters/TransportFeedbackManager.hpp"

namespace stfuzz::adapters {

TransportFeedbackManager::TransportFeedbackManager(QObject* parent)
    : QObject(parent) {}

void TransportFeedbackManager::onTransportEvent(const TransportEvent& event) {
  lastError = event;
  updateStatus(event);

  if (event.errorCode != 0) {
    emit transportErrorReported(event);
  }

  const TransmissionResult result = createTransmissionResult(event);
  emit commandTransmissionResult(result);
}

void TransportFeedbackManager::updateStatus(const TransportEvent& event) {
  const TransportStatus newStatus =
      (event.errorCode == 0) ? TransportStatus::Active : TransportStatus::Error;

  if (currentStatus != newStatus) {
    currentStatus = newStatus;
    emit transportStatusChanged(currentStatus);
  }
}

TransmissionResult TransportFeedbackManager::createTransmissionResult(
    const TransportEvent& event) const {
  TransmissionResult result;
  result.requestId = 0;
  result.bytesAccepted = event.bytesTransferred;
  result.status = (event.errorCode == 0) ? CommandStatus::Transmitted
                                         : CommandStatus::TransportError;
  result.errorDescription = event.description;
  return result;
}

}  // namespace stfuzz::adapters
