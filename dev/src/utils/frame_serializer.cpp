#include "utils/frame_serializer.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "utils/ieee802154_protocol.hpp"

namespace stfuzz::utils::serializer {

std::vector<uint8_t> SerializeFrame(const ProtocolModel& protocol,
                                    const void* frame_model) {
  if (frame_model == nullptr ||
      (protocol.fields == nullptr && protocol.num_fields != 0)) {
    return {};
  }

  std::vector<uint8_t> out;

  for (size_t i = 0; i < protocol.num_fields; ++i) {
    const FieldSpec& field = protocol.fields[i];
    if (field.is_present != nullptr && !field.is_present(frame_model)) {
      continue;
    }
    if (field.write_bytes == nullptr) {
      return {};
    }
    field.write_bytes(frame_model, out);
  }

  return out;
}

}  // namespace stfuzz::utils::serializer
