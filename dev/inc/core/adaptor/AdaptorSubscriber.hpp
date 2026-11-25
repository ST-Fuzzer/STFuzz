#pragma once

#include <cstdint>
#include <variant>
#include <vector>

#include "core/adaptor/AdaptorConfigInterface.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace adaptor {

struct AdaptorEvent {
  enum class Type : uint8_t { kFrame, kFeedback };
  using Data = std::variant<std::vector<uint8_t>, ConfigMap>;

  Type type = Type::kFeedback;
  stfuzz::utils::ErrorCode code = stfuzz::utils::ErrorCode::kOk;
  Data data;
};

class IAdaptorSubscriber {
 public:
  virtual ~IAdaptorSubscriber() = default;

  virtual void Update(const AdaptorEvent& event) = 0;
};

}  // namespace adaptor
}  // namespace core
}  // namespace stfuzz
