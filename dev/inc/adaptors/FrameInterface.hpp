#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

using FrameCallback =
    std::function<void(const std::vector<uint8_t>& raw_frame)>;

class IFrame {
 public:
  virtual ~IFrame() = default;

  virtual utils::ErrorCode SendFrame(const std::vector<uint8_t>& raw_frame) = 0;
  virtual void RegisterFrameCallback(FrameCallback callback) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
