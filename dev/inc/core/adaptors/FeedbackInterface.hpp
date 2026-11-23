#pragma once

#include "adaptor/FeedbackSubscriber.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

class IFeedback {
 public:
  virtual ~IFeedback() = default;

  virtual utils::ErrorCode Subscribe(IFeedbackSubscriber* subscriber) = 0;
  virtual void Unsubscribe(IFeedbackSubscriber* subscriber) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
