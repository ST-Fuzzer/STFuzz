#pragma once

#include "core/adaptor/AdaptorSubscriber.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace adaptor {

class IAdaptor {
 public:
  virtual ~IAdaptor() = default;

  [[nodiscard]] virtual stfuzz::utils::ErrorCode Send(
      const AdaptorMessage& message) = 0;

  [[nodiscard]] virtual stfuzz::utils::ErrorCode Subscribe(
      IAdaptorSubscriber* subscriber) = 0;
  virtual void Unsubscribe(IAdaptorSubscriber* subscriber) = 0;
};

}  // namespace adaptor
}  // namespace core
}  // namespace stfuzz
