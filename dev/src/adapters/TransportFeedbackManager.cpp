#include "adapters/TransportFeedbackManager.hpp"

#include <QtCore/qtmetamacros.h>

#include <QObject>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

TransportFeedbackManager::TransportFeedbackManager(QObject* parent)
    : QObject(parent) {}

void TransportFeedbackManager::onTransportEvent(const TransportEvent& event) {
  lastError = event;
  updateStatus(event);

  if (event.errorCode != 0) {
    Q_EMIT transportErrorReported(event);
  }

  const TransmissionResult result = createTransmissionResult(event);
  Q_EMIT commandTransmissionResult(result);
}

void TransportFeedbackManager::updateStatus(const TransportEvent& event) {
  const TransportStatus new_status =
      (event.errorCode == 0) ? TransportStatus::Active : TransportStatus::Error;

  if (currentStatus != new_status) {
    currentStatus = new_status;
    Q_EMIT transportStatusChanged(currentStatus);
  }
}

TransmissionResult TransportFeedbackManager::createTransmissionResult(
    const TransportEvent& event) {
  TransmissionResult result;
  result.requestId = event.requestIdValue;
  result.bytesAccepted = event.bytesTransferred;
  result.status = (event.errorCode == 0) ? CommandStatus::Transmitted
                                         : CommandStatus::TransportError;
  result.errorDescription = event.description;
  return result;
}

}  // namespace stfuzz::adapters
