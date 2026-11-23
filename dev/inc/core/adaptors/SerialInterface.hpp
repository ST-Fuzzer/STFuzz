#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

class ISerial {
 public:
  virtual ~ISerial() = default;

  virtual utils::ErrorCode Open() = 0;
  virtual void Close() = 0;
  virtual bool IsOpen() const = 0;
  virtual utils::ErrorCode Write(const std::vector<uint8_t>& data) = 0;
  virtual utils::ErrorCode Read(std::vector<uint8_t>& buffer,
                                size_t max_bytes) = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
