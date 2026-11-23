#pragma once

#include <string>
#include <unordered_map>

namespace stfuzz {
namespace adaptor {

using FeedbackMap = std::unordered_map<std::string, std::string>;

class IFeedbackSubscriber {
 public:
  virtual ~IFeedbackSubscriber() = default;

  virtual void Update(const FeedbackMap& feedback) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
