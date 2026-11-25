#pragma once

#include <string>
#include <unordered_map>

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace adaptors {

using ConfigMap = std::unordered_map<std::string, std::string>;

class IAdaptor {
 public:
  virtual ~IAdaptor() = default;

  [[nodiscard]] virtual bool HasKey(const std::string& key) const = 0;
  [[nodiscard]] virtual stfuzz::utils::ErrorCode GetValue(
      const std::string* value, const std::string& key);
  [[nodiscard]] virtual const ConfigMap& Entries() const = 0;
};

}  // namespace adaptors
}  // namespace core
}  // namespace stfuzz
