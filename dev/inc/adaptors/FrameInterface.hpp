#pragma once

#include <cstdint>
#include <vector>

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

class IFrameObserver {
 public:
  virtual ~IFrameObserver() = default;

  virtual void OnFrame(const std::vector<uint8_t>& raw_frame) = 0;
};

class IFrame {
 public:
  virtual ~IFrame() = default;

  virtual utils::ErrorCode SendFrame(const std::vector<uint8_t>& raw_frame) = 0;
  virtual void Subscribe(IFrameObserver* observer) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
