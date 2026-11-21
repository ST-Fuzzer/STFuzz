#pragma once

#include <QByteArray>
#include <QObject>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

class CommandManager : public QObject {
  Q_OBJECT

 public:
  explicit CommandManager(QObject* parent = nullptr);
  ~CommandManager() override = default;

  RequestId submitCommand(const Command& command);

 signals:
  void commandReady(const QByteArray& data, RequestId id);
  void contextCreated(const CommandContext& context);

 private:
  static bool validateCommand(const Command& command);
  static QByteArray encodeCommand(const Command& command);
  static CommandContext createContext(const Command& command, RequestId id);

  RequestId nextRequestId{1};
};

}  // namespace stfuzz::adapters
