#pragma once

#include <string_view>

namespace stfuzz {
namespace core {
namespace adaptor {
namespace data {

struct Entry {
  std::string_view key;
  std::string_view value;
};

}  // namespace data
}  // namespace adaptor
}  // namespace core
}  // namespace stfuzz
