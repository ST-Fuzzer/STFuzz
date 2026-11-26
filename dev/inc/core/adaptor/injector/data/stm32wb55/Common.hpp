#pragma once

#include <array>

#include "core/adaptor/injector/data/Entry.hpp"

namespace stfuzz {
namespace core {
namespace adaptor {
namespace data {
namespace stm32wb55 {
namespace common {

inline constexpr std::array kEntries {
  Entry{"vendor_id", "0x0483"},
  Entry{"product_id", "0x374B"},
  Entry{"device_name", "P-NUCLEO-WB55"},

  Entry{"baud_rate", "921600"},
  Entry{"data_bits", "8"},
  Entry{"parity", "none"},
  Entry{"stop_bits", "1"},
};

}  // namespace common
}  // namespace stm32wb55
}  // namespace data
}  // namespace adaptor
}  // namespace core
}  // namespace stfuzz
