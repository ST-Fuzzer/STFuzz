#pragma once

#include <array>

#include "core/adaptor/injector/data/Entry.hpp"

namespace stfuzz {
namespace core {
namespace adaptor {
namespace data {
namespace stm32wb55 {
namespace ieee802154 {

inline constexpr std::array kEntries {
  Entry{"default_channel", "15"},
  Entry{"max_payload_size", "127"},
};

}  // namespace ieee802154
}  // namespace stm32wb55
}  // namespace data
}  // namespace adaptor
}  // namespace core
}  // namespace stfuzz
