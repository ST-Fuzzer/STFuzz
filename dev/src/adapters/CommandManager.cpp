#include "adapters/CommandManager.hpp"

#include <QDataStream>
#include <QDateTime>
#include <QIODevice>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "adapters/STFuzzTypes.hpp"

namespace stfuzz::adapters {

CommandManager::CommandManager(QObject* parent) : QObject(parent) {}

RequestId CommandManager::submitCommand(const Command& command) {
  if (!validateCommand(command)) {
    return 0;
  }

  const RequestId id = nextRequestId++;
  const QByteArray data = encodeCommand(command);
  const CommandContext context = createContext(command, id);

  emit contextCreated(context);
  emit commandReady(data, id);

  return id;
}

bool CommandManager::validateCommand(const Command& command) {
  if (command.type < 0) {
    return false;
  }

  if (command.parameters.contains(QStringLiteral("timeout_ms"))) {
    bool ok = false;
    const int timeout =
        command.parameters.value(QStringLiteral("timeout_ms")).toInt(&ok);
    if (!ok || timeout <= 0) {
      return false;
    }
  }

  return true;
}

QByteArray CommandManager::encodeCommand(const Command& command) {
  QByteArray buffer;
  QDataStream stream(&buffer, QIODevice::WriteOnly);
  stream.setByteOrder(QDataStream::BigEndian);

  stream << static_cast<qint32>(command.type);

  if (command.parameters.contains(QStringLiteral("raw_payload"))) {
    const QByteArray raw_payload =
        command.parameters.value(QStringLiteral("raw_payload")).toByteArray();
    stream << static_cast<quint32>(raw_payload.size());
    buffer.append(raw_payload);
  } else {
    stream << static_cast<quint32>(0);
  }

  return buffer;
}

CommandContext CommandManager::createContext(const Command& command,
                                             RequestId id) {
  CommandContext context;
  context.requestId = id;
  context.commandType = command.type;

  if (command.parameters.contains(QStringLiteral("expected_response"))) {
    context.expectedResponse =
        command.parameters.value(QStringLiteral("expected_response")).toInt();
  } else {
    context.expectedResponse = command.type;
  }

  if (command.parameters.contains(QStringLiteral("timeout_ms"))) {
    context.timeoutMs =
        command.parameters.value(QStringLiteral("timeout_ms")).toInt();
  } else {
    context.timeoutMs = 1000;
  }

  context.createdAt = QDateTime::currentDateTimeUtc();
  return context;
}

}  // namespace stfuzz::adapters
