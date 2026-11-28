#include "core/main_controller/MainController.hpp"

#include <cstdint>
#include <string>

#include "core/adaptor/AdaptorInterface.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace core {
namespace controller {

MainController::MainController(adaptor::IAdaptor& adaptor)
    : adaptor_(adaptor) {}

stfuzz::utils::ErrorCode MainController::Run(std::uint32_t num_tests,
                                             const std::string& payload) {
  adaptor::AdaptorMessage message;
  message.data = payload;

  for (std::uint32_t test_index = 0; test_index < num_tests; ++test_index) {
    const stfuzz::utils::ErrorCode result = adaptor_.Send(message);
    if (!stfuzz::utils::IsOk(result)) {
      return result;
    }
  }

  return stfuzz::utils::ErrorCode::kOk;
}

}  // namespace controller
}  // namespace core
}  // namespace stfuzz
