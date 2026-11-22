#pragma once

#include <cstdint>
#include <vector>

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

struct StatusReport {
  utils::ErrorCode code = utils::ErrorCode::kOk;
  std::vector<uint8_t> payload;
};

class IStatusObserver {
 public:
  virtual ~IStatusObserver() = default;

  virtual void OnStatus(const StatusReport& report) = 0;
};

class IStatus {
 public:
  virtual ~IStatus() = default;

  virtual void Subscribe(IStatusObserver* observer) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
