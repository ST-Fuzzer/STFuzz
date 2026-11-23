#pragma once

#include "core/adaptors/FeedbackSubscriber.hpp"
#include "core/adaptors/StatusInterface.hpp"

namespace stfuzz {
namespace adaptor {

enum class DUTMessageType { kStatus, kFeedback, kUnknown };

struct DUTMessage {
  DUTMessageType type = DUTMessageType::kUnknown;

  StatusReport status;
  FeedbackMap feedback;
};

}  // namespace adaptor
}  // namespace stfuzz
