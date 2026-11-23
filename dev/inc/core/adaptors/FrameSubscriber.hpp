#pragma once

#include <cstdint>
#include <vector>

namespace stfuzz {
namespace adaptor {

class IFrameSubscriber {
 public:
  virtual ~IFrameSubscriber() = default;

  virtual void Update(const std::vector<uint8_t>& raw_frame) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
