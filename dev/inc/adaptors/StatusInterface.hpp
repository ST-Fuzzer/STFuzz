#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

struct StatusReport {
  utils::ErrorCode code = utils::ErrorCode::kOk;
  std::vector<uint8_t> payload;
};

using StatusCallback = std::function<void(const StatusReport& report)>;

class IStatus {
 public:
  virtual ~IStatus() = default;

  virtual void RegisterStatusCallback(StatusCallback callback) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
