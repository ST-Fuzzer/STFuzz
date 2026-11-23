#include "core/adaptors/dut_adaptor/StatusManager.hpp"

#include <algorithm>

#include "core/adaptors/FeedbackSubscriber.hpp"
#include "core/adaptors/StatusInterface.hpp"
#include "core/adaptors/StatusSubscriber.hpp"
#include "core/adaptors/dut_adaptor/DUTMessage.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz::adaptor {

// Status subscribers

utils::ErrorCode StatusManager::Subscribe(IStatusSubscriber* subscriber) {
  if (subscriber == nullptr) {
    return utils::ErrorCode::kInvalidArgument;
  }

  if (std::find(status_subscribers_.begin(), status_subscribers_.end(),
                subscriber) == status_subscribers_.end()) {
    status_subscribers_.push_back(subscriber);
  }

  return utils::ErrorCode::kOk;
}

void StatusManager::Unsubscribe(IStatusSubscriber* subscriber) {
  status_subscribers_.erase(std::remove(status_subscribers_.begin(),
                                        status_subscribers_.end(), subscriber),
                            status_subscribers_.end());
}

// Feedback subscribers

utils::ErrorCode StatusManager::Subscribe(IFeedbackSubscriber* subscriber) {
  if (subscriber == nullptr) {
    return utils::ErrorCode::kInvalidArgument;
  }

  if (std::find(feedback_subscribers_.begin(), feedback_subscribers_.end(),
                subscriber) == feedback_subscribers_.end()) {
    feedback_subscribers_.push_back(subscriber);
  }

  return utils::ErrorCode::kOk;
}

void StatusManager::Unsubscribe(IFeedbackSubscriber* subscriber) {
  feedback_subscribers_.erase(
      std::remove(feedback_subscribers_.begin(), feedback_subscribers_.end(),
                  subscriber),
      feedback_subscribers_.end());
}

// Message classification and routing

utils::ErrorCode StatusManager::HandleMessage(const DUTMessage& message) {
  switch (message.type) {
    case DUTMessageType::kStatus:
      PublishStatus(message.status);
      return utils::ErrorCode::kOk;

    case DUTMessageType::kFeedback:
      PublishFeedback(message.feedback);
      return utils::ErrorCode::kOk;

    default:
      return utils::ErrorCode::kDecodeFailed;
  }
}

// Publishing

void StatusManager::PublishStatus(const StatusReport& report) {
  const auto subscribers = status_subscribers_;

  for (auto* subscriber : subscribers) {
    if (subscriber != nullptr) {
      subscriber->Update(report);
    }
  }
}

void StatusManager::PublishFeedback(const FeedbackMap& feedback) {
  const auto subscribers = feedback_subscribers_;

  for (auto* subscriber : subscribers) {
    if (subscriber != nullptr) {
      subscriber->Update(feedback);
    }
  }
}

}  // namespace stfuzz::adaptor
