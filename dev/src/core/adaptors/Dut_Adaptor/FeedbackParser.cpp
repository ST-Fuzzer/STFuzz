#include "core/adaptors/dut_adaptor/FeedbackParser.hpp"

#include <cstdint>
#include <vector>

#include "core/adaptors/dut_adaptor/DUTMessage.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz::adaptor {

utils::ErrorCode FeedbackParser::ProcessData(
    const std::vector<uint8_t>& data, std::vector<DUTMessage>& messages) {
  messages.clear();

  if (data.empty()) {
    return utils::ErrorCode::kOk;
  }

  buffer_.insert(buffer_.end(), data.begin(), data.end());

  return utils::ErrorCode::kOk;
}

}  // namespace stfuzz::adaptor
