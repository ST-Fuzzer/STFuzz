#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "utils/ieee802154_protocol.hpp"

namespace stfuzz::utils {

// Serializes `frame_model` according to `protocol` and returns the raw bytes.
// Returns `std::nullopt` if `frame_model` is null or a field writer is null.
[[nodiscard]] std::optional<std::vector<uint8_t>> SerializeFrame(
    const ProtocolModel& protocol, const void* frame_model);
}  // namespace stfuzz::utils
