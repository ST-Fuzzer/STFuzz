#pragma once

#include <cstdint>
#include <vector>

#include "adaptor/FrameSubscriber.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

class IFrame {
 public:
  virtual ~IFrame() = default;

  virtual utils::ErrorCode SendFrame(const std::vector<uint8_t>& raw_frame) = 0;
  virtual utils::ErrorCode Subscribe(IFrameSubscriber* subscriber) = 0;
  virtual void Unsubscribe(IFrameSubscriber* subscriber) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
