#pragma once

#include <cstdint>
#include <vector>

#include "core/adaptor/AdaptorSubscriber.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace adaptor {

class IAdaptor {
 public:
  virtual ~IAdaptor() = default;

  [[nodiscard]] virtual stfuzz::utils::ErrorCode Send(
      const std::vector<uint8_t>& frame) const = 0;
  [[nodiscard]] virtual stfuzz::utils::ErrorCode Rest() = 0;

  [[nodiscard]] virtual stfuzz::utils::ErrorCode Subscriber(
      IAdaptorSubscriber* subscriber) = 0;
  virtual void Unsubscribe(IAdaptorSubscriber* subscriber) = 0;
};

}  // namespace adaptor
}  // namespace core
}  // namespace stfuzz
