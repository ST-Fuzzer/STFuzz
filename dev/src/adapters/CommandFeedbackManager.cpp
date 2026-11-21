#include "adapters/CommandFeedbackManager.hpp"

#include <QtCore/qtmetamacros.h>

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QtGlobal>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

CommandFeedbackManager::CommandFeedbackManager(QObject* parent)
    : QObject(parent) {}

void CommandFeedbackManager::cancelAllPendingCommands() {
  for (auto* timer : timeoutTimers) {
    timer->stop();
    timer->deleteLater();
  }
  timeoutTimers.clear();
  pendingCommands.clear();
}

void CommandFeedbackManager::contextCreated(const CommandContext& context) {
  pendingCommands.insert(context.requestId, context);

  auto* timer = new QTimer(this);
  timer->setSingleShot(true);

  connect(timer, &QTimer::timeout, this,
          [this, id = context.requestId] { onCommandTimeout(id); });

  timeoutTimers.insert(context.requestId, timer);
  timer->start(context.timeoutMs);
}

void CommandFeedbackManager::onTransmissionResult(
    const TransmissionResult& result) {
  if (result.status == CommandStatus::TransportError) {
    completeCommand(result.requestId, CommandResult{
                                          result.requestId,
                                          0,
                                          CommandStatus::TransportError,
                                          QByteArray(),
                                          result.errorDescription,
                                      });
  }
}

void CommandFeedbackManager::onCommandResponse(
    const CommandResponse& response) {
  const RequestId id = matchResponse(response);
  if (id != 0) {
    completeCommand(id, CommandResult{
                            id,
                            response.responseType,
                            CommandStatus::Success,
                            response.payload,
                            QString(),
                        });
  }
}

void CommandFeedbackManager::onCommandTimeout(RequestId id) {
  if (!pendingCommands.contains(id)) {
    return;
  }

  const CommandContext ctx = pendingCommands.take(id);
  if (timeoutTimers.contains(id)) {
    auto* timer = timeoutTimers.take(id);
    timer->stop();
    timer->deleteLater();
  }

  CommandTimeoutEvent event;
  event.requestId = id;
  event.commandType = ctx.commandType;
  event.timeoutMs = ctx.timeoutMs;
  event.occurredAt = QDateTime::currentDateTimeUtc();

  Q_EMIT commandTimeoutDetected(event);
}

RequestId CommandFeedbackManager::matchResponse(
    const CommandResponse& response) const {
  if (pendingCommands.contains(response.correlationId)) {
    return response.correlationId;
  }
  return 0;
}

void CommandFeedbackManager::completeCommand(RequestId id,
                                             const CommandResult& result) {
  pendingCommands.remove(id);
  if (timeoutTimers.contains(id)) {
    auto* timer = timeoutTimers.take(id);
    timer->stop();
    timer->deleteLater();
  }
  Q_EMIT commandResultReady(result);
}

}  // namespace stfuzz::adapters
