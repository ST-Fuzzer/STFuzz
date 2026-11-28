#pragma once

#include <cstdint>
#include <string>

#include "core/adaptor/AdaptorInterface.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace controller {


class MainController {
 public:

  explicit MainController(adaptor::IAdaptor& adaptor);
  [[nodiscard]] stfuzz::utils::ErrorCode Run(std::uint32_t num_tests,
                                             const std::string& payload);

 private:
  adaptor::IAdaptor& adaptor_;
};

}  // namespace controller
}  // namespace core
}  // namespace stfuzz
