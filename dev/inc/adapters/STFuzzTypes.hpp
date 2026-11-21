#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QVariantMap>
#include <cstdint>

namespace stfuzz::adapters {

using RequestId = quint64;
using TestcaseId = quint64;

enum class ConnectionState { Disconnected, Connecting, Connected, Error };
enum class TransportStatus { Disconnected, Active, Error };
enum class CommandStatus {
  Pending,
  Transmitted,
  Success,
  Timeout,
  TransportError,
  EncodingError
};
enum class DUTHealthState {
  Unknown,
  Responsive,
  SuspectedUnresponsive,
  FaultReported,
  ResetObserved
};
enum class FeedbackSource {
  CommandFeedbackManager,
  TelemetryProcessor,
  HealthMonitor,
  TransportFeedbackManager,
  DeviceManager
};
enum class FeedbackType {
  COMMAND_RESULT,
  TELEMETRY_RECORD,
  TELEMETRY_EVENT,
  HEALTH_EVENT,
  TRANSPORT_ERROR,
  PARSING_ERROR,
  CONNECTION_STATE
};

struct Command {
  int type{0};
  QVariantMap parameters;
};

struct CommandContext {
  RequestId requestId{0};
  int commandType{0};
  int expectedResponse{0};
  int timeoutMs{1000};
  QDateTime createdAt;
};

struct CommandResult {
  RequestId requestId{0};
  int commandType{0};
  CommandStatus status{CommandStatus::Pending};
  QByteArray responseData;
  QString errorDescription;
};

struct CommandTimeoutEvent {
  RequestId requestId{0};
  int commandType{0};
  int timeoutMs{0};
  QDateTime occurredAt;
};

struct CommandResponse {
  int responseType{0};
  QByteArray payload;
  RequestId correlationId{0};
  QDateTime receivedAt;
};

struct TransportEvent {
  int eventType{0};
  RequestId requestIdValue{0};
  QDateTime timestamp;
  qint64 bytesTransferred{0};
  int errorCode{0};
  QString description;
};

struct TransmissionResult {
  RequestId requestId{0};
  CommandStatus status{CommandStatus::Pending};
  qint64 bytesAccepted{0};
  QString errorDescription;
};

struct DecodedMessage {
  int messageType{0};
  quint16 sequenceNumber{0};
  QByteArray payload;
  QDateTime receivedAt;
};

struct TelemetryRecord {
  int recordType{0};
  quint16 sequenceNumber{0};
  QDateTime timestamp;
  QVariantMap values;
};

struct TelemetryEvent {
  int eventType{0};
  QDateTime timestamp;
  int severity{0};
  QVariantMap details;
};

struct HealthEvent {
  int eventType{0};
  DUTHealthState healthState{DUTHealthState::Unknown};
  QDateTime timestamp;
  QString description;
  QVariantMap evidence;
};

struct ParsingError {
  int errorType{0};
  QDateTime timestamp;
  QString description;
  QByteArray rawFrame;
};

struct DUTFeedback {
  TestcaseId testcaseId{0};
  quint64 sequenceNumber{0};
  QDateTime timestamp;
  FeedbackSource source;
  FeedbackType type;
  QVariant payload;
};

}  // namespace stfuzz::adapters
