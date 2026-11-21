#pragma once

#include <QHash>
#include <QObject>
#include <QTimer>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

class CommandFeedbackManager : public QObject {
  Q_OBJECT

 public:
  explicit CommandFeedbackManager(QObject* parent = nullptr);
  ~CommandFeedbackManager() override = default;

  void cancelAllPendingCommands();

 public slots:
  void contextCreated(const CommandContext& context);
  void onTransmissionResult(const TransmissionResult& result);
  void onCommandResponse(const CommandResponse& response);

 signals:
  void commandResultReady(const CommandResult& result);
  void commandTimeoutDetected(const CommandTimeoutEvent& event);

 private slots:
  void onCommandTimeout(RequestId id);

 private:
  RequestId matchResponse(const CommandResponse& response) const;
  void completeCommand(RequestId id, const CommandResult& result);

  QHash<RequestId, CommandContext> pendingCommands;
  QHash<RequestId, QTimer*> timeoutTimers;
};

}  // namespace stfuzz::adapters
