#pragma once

#include <cstdint>
#include <vector>

#include "utils/ieee802154_protocol.hpp"

namespace stfuzz::utils::serializer {

/**
 * @brief Serializes a structured protocol frame model into raw on-air bytes
 *        for transmission or fuzzing injection.
 *
 * @param protocol Protocol descriptor defining the ordered fields and writers.
 * @param frame_model Pointer to the populated frame structure to serialize.
 * @return Serialized byte vector, or an empty vector on invalid input.
 */
std::vector<uint8_t> SerializeFrame(const ProtocolModel& protocol,
                                    const void* frame_model);

}  // namespace stfuzz::utils::serializer
