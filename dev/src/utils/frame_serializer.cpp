#include "utils/frame_serializer.hpp"

namespace stfuzz::utils {

std::optional<std::vector<uint8_t>> SerializeFrame(
    const ProtocolModel& protocol, const void* frame_model) {
  if (frame_model == nullptr ||
      (protocol.fields == nullptr && protocol.num_fields != 0)) {
    return std::nullopt;
  }

  std::vector<uint8_t> out;

  for (size_t i = 0; i < protocol.num_fields; ++i) {
    const FieldSpec& field = protocol.fields[i];
    if (field.is_present != nullptr && !field.is_present(frame_model)) {
      continue;
    }
    if (field.write_bytes == nullptr) {
      return std::nullopt;
    }
    field.write_bytes(frame_model, out);
  }

  return out;
}

}  // namespace stfuzz::utils
