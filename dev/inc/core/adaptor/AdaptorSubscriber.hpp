#pragma once

#include <string>
#include <variant>

#include "core/adaptor/AdaptorConfigInterface.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace adaptor {

struct AdaptorMessage {
  using Data = std::variant<std::string, ConfigMap>;

  stfuzz::utils::ErrorCode code = stfuzz::utils::ErrorCode::kOk;
  Data data = std::string{};
};

class IAdaptorSubscriber {
 public:
  virtual ~IAdaptorSubscriber() = default;

  virtual void UpdateMessageReceived(const AdaptorMessage& message) = 0;
};

}  // namespace adaptor
}  // namespace core
}  // namespace stfuzz
