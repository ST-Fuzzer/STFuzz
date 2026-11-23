#pragma once

#include <cstdint>
#include <vector>

#include "adaptor/StatusSubscriber.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

struct StatusReport {
  utils::ErrorCode code = utils::ErrorCode::kOk;
  std::vector<uint8_t> payload;
};

class IStatus {
 public:
  virtual ~IStatus() = default;

  virtual utils::ErrorCode Subscribe(IStatusSubscriber* subscriber) = 0;
  virtual void Unsubscribe(IStatusSubscriber* subscriber) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
