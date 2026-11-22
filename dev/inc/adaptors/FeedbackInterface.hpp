#pragma once

#include <string>
#include <unordered_map>

namespace stfuzz {
namespace adaptor {

using FeedbackMap = std::unordered_map<std::string, std::string>;

class IFeedbackObserver {
 public:
  virtual ~IFeedbackObserver() = default;

  virtual void OnFeedback(const FeedbackMap& feedback) = 0;
};

class IFeedback {
 public:
  virtual ~IFeedback() = default;

  virtual void Subscribe(IFeedbackObserver* observer) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
