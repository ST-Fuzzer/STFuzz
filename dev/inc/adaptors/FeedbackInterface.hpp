#pragma once

#include <functional>
#include <string>
#include <unordered_map>

namespace stfuzz {
namespace adaptor {

using FeedbackMap = std::unordered_map<std::string, std::string>;
using FeedbackCallback = std::function<void(const FeedbackMap& feedback)>;

class IFeedback {
 public:
  virtual ~IFeedback() = default;

  virtual void RegisterFeedbackCallback(FeedbackCallback callback) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
