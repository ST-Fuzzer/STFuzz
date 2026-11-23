#pragma once

#include <vector>

#include "core/adaptors/FeedbackInterface.hpp"
#include "core/adaptors/StatusInterface.hpp"
#include "core/adaptors/dut_adaptor/DUTMessage.hpp"

namespace stfuzz {
namespace adaptor {

class StatusManager : public IStatus, public IFeedback {
 public:
  utils::ErrorCode Subscribe(IStatusSubscriber* subscriber) override;
  void Unsubscribe(IStatusSubscriber* subscriber) override;

  utils::ErrorCode Subscribe(IFeedbackSubscriber* subscriber) override;
  void Unsubscribe(IFeedbackSubscriber* subscriber) override;

  utils::ErrorCode HandleMessage(const DUTMessage& message);

 private:
  void PublishStatus(const StatusReport& report);
  void PublishFeedback(const FeedbackMap& feedback);

  std::vector<IStatusSubscriber*> status_subscribers_;
  std::vector<IFeedbackSubscriber*> feedback_subscribers_;
};

}  // namespace adaptor
}  // namespace stfuzz
