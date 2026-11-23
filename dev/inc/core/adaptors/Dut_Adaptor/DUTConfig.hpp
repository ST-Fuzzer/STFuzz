#pragma once

#include <optional>
#include <string>

#include "core/adaptors/ConfigInterface.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace adaptor {

class DUTConfig {
 public:
  utils::ErrorCode SetParameter(const std::string& key,
                                const std::string& value);
  std::optional<std::string> GetValue(const std::string& key) const;
  bool HasKey(const std::string& key) const;
  utils::ErrorCode Validate() const;

 private:
  ConfigMap parameters_;
};

}  // namespace adaptor
}  // namespace stfuzz
