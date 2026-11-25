#pragma once

#include <cstdint>

namespace stfuzz {
namespace utils {

enum class ErrorCode : uint8_t {
  kOk = 0,

  // General / Common (1 - 99)
  kUnknown = 1,
  kInvalidArgument,
  kNotInitialized,
  kTimeout,

  // Adaptors: Connection & Transport (100 - 129)
  kNotConnected = 100,
  kAlreadyConnected,
  kDisconnected,
  kPortNotFound,
  kPortOpenFailed,
  kPortConfigFailed,
  kWriteFailed,
  kReadFailed,
  kHandshakeFailed,

  // Adaptors: Config (130 - 149)
  kConfigFileNotFound = 130,
  kConfigParseFailed,
  kConfigKeyNotFound,
  kInvalidConfig,

  // Adaptors: Frame TX/RX & Parsing (150 - 179)
  kInvalidFrame = 150,
  kTxQueueFull,
  kTxRejected,
  kRxBufferOverflow,
  kDecodeFailed,
  kCallbackNotSet,

  // Adaptors: Target / DUT & Control (180 - 199)
  kResetFailed = 180,
  kTargetHalted,
  kTargetMemoryAccessFailed,
};

constexpr bool IsOk(ErrorCode code) noexcept { return code == ErrorCode::kOk; }

}  // namespace utils
}  // namespace stfuzz
