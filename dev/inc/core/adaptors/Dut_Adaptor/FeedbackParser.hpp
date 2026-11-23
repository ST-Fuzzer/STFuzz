#pragma once

#include <cstdint>
#include <vector>

#include "core/adaptors/dut_adaptor/DUTMessage.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

class FeedbackParser {
 public:
  utils::ErrorCode ProcessData(const std::vector<uint8_t>& data,
                               std::vector<DUTMessage>& messages);

 private:
  std::vector<uint8_t> buffer_;
};

}  // namespace adaptor
}  // namespace stfuzz
