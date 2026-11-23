#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

using ConfigMap = std::unordered_map<std::string, std::string>;

class IConfig {
 public:
  virtual ~IConfig() = default;

  virtual utils::ErrorCode SetParameter(const std::string& key,
                                        const std::string& value) = 0;
  virtual std::optional<std::string> GetValue(const std::string& key) const = 0;
  virtual bool HasKey(const std::string& key) const = 0;
  virtual utils::ErrorCode Validate() const = 0;
};

}  // namespace adaptor
}  // namespace stfuzz
