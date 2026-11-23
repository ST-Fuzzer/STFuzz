#include "core/adaptors/dut_adaptor/SerialLink.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include "utils/ErrorCode.hpp"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#endif

namespace stfuzz::adaptor {

#ifdef _WIN32

namespace {

utils::ErrorCode MapOpenError(DWORD error) {
  if (error == ERROR_FILE_NOT_FOUND ||
      error == ERROR_PATH_NOT_FOUND ||
      error == ERROR_INVALID_NAME) {
    return utils::ErrorCode::kPortNotFound;
  }

  return utils::ErrorCode::kPortOpenFailed;
}

}  // namespace

#endif

SerialLink::SerialLink(SerialPortSettings settings)
    : settings_(std::move(settings)) {}

SerialLink::~SerialLink() {
  Close();
}

// Open serial connection

utils::ErrorCode SerialLink::Open() {
  if (IsOpen()) {
    return utils::ErrorCode::kAlreadyConnected;
  }

  if (settings_.port_name.empty() || settings_.baud_rate == 0U) {
    return utils::ErrorCode::kInvalidArgument;
  }

#ifdef _WIN32

  std::string device_path = settings_.port_name;

  // Add Windows COM device prefix if necessary.

  if (device_path.rfind("\\\\.\\", 0U) != 0U) {
    device_path = "\\\\.\\" + device_path;
  }

  HANDLE handle = CreateFileA(
      device_path.c_str(),
      GENERIC_READ | GENERIC_WRITE,
      0,
      nullptr,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,
      nullptr);

  if (handle == INVALID_HANDLE_VALUE) {
    return MapOpenError(GetLastError());
  }

  // Retrieve current serial configuration.

  DCB dcb{};

  dcb.DCBlength = sizeof(DCB);

  if (GetCommState(handle, &dcb) == FALSE) {
    CloseHandle(handle);
    return utils::ErrorCode::kPortConfigFailed;
  }

  // Configure UART: 115200 8N1.

  dcb.BaudRate = settings_.baud_rate;
  dcb.ByteSize = 8U;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;

  dcb.fBinary = TRUE;
  dcb.fParity = FALSE;

  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDsrSensitivity = FALSE;

  dcb.fOutX = FALSE;
  dcb.fInX = FALSE;

  dcb.fRtsControl = RTS_CONTROL_DISABLE;

  if (SetCommState(handle, &dcb) == FALSE) {
    CloseHandle(handle);
    return utils::ErrorCode::kPortConfigFailed;
  }

  // Configure read/write timeouts.

  COMMTIMEOUTS timeouts{};

  timeouts.ReadIntervalTimeout = MAXDWORD;
  timeouts.ReadTotalTimeoutMultiplier = 0U;
  timeouts.ReadTotalTimeoutConstant = 0U;

  timeouts.WriteTotalTimeoutMultiplier = 0U;
  timeouts.WriteTotalTimeoutConstant = 1000U;

  if (SetCommTimeouts(handle, &timeouts) == FALSE) {
    CloseHandle(handle);
    return utils::ErrorCode::kPortConfigFailed;
  }

  // Clear any previous buffered data.

  PurgeComm(handle, PURGE_RXCLEAR | PURGE_TXCLEAR);

  handle_ = handle;

  return utils::ErrorCode::kOk;

#else

  return utils::ErrorCode::kNotInitialized;

#endif
}

// Close serial connection

void SerialLink::Close() {

#ifdef _WIN32

  if (handle_ == nullptr) {
    return;
  }

  HANDLE handle = static_cast<HANDLE>(handle_);

  PurgeComm(
      handle,
      PURGE_RXABORT |
      PURGE_RXCLEAR |
      PURGE_TXABORT |
      PURGE_TXCLEAR);

  CloseHandle(handle);

  handle_ = nullptr;

#else

  handle_ = nullptr;

#endif
}

// Check connection state

bool SerialLink::IsOpen() const {
  return handle_ != nullptr;
}

// Transmit bytes

utils::ErrorCode SerialLink::Write(
    const std::vector<uint8_t>& data) {

  if (!IsOpen()) {
    return utils::ErrorCode::kNotConnected;
  }

  if (data.empty()) {
    return utils::ErrorCode::kOk;
  }

#ifdef _WIN32

  HANDLE handle = static_cast<HANDLE>(handle_);

  size_t offset = 0U;

  while (offset < data.size()) {

    const size_t remaining = data.size() - offset;

    const size_t chunk_size = std::min(
        remaining,
        static_cast<size_t>(MAXDWORD));

    DWORD bytes_written = 0U;

    if (WriteFile(
            handle,
            data.data() + offset,
            static_cast<DWORD>(chunk_size),
            &bytes_written,
            nullptr) == FALSE) {

      return utils::ErrorCode::kWriteFailed;
    }

    if (bytes_written == 0U) {
      return utils::ErrorCode::kWriteFailed;
    }

    offset += bytes_written;
  }

  return utils::ErrorCode::kOk;

#else

  return utils::ErrorCode::kNotInitialized;

#endif
}

// Receive raw bytes

utils::ErrorCode SerialLink::Read(
    std::vector<uint8_t>& buffer,
    size_t max_bytes) {

  buffer.clear();

  if (!IsOpen()) {
    return utils::ErrorCode::kNotConnected;
  }

  if (max_bytes == 0U) {
    return utils::ErrorCode::kOk;
  }

#ifdef _WIN32

  HANDLE handle = static_cast<HANDLE>(handle_);

  DWORD communication_errors = 0U;

  COMSTAT communication_status{};

  if (ClearCommError(
          handle,
          &communication_errors,
          &communication_status) == FALSE) {

    return utils::ErrorCode::kReadFailed;
  }

  constexpr DWORD read_error_flags =
      CE_RXOVER | CE_OVERRUN | CE_FRAME | CE_RXPARITY | CE_BREAK;

  if ((communication_errors & read_error_flags) != 0U) {
    return utils::ErrorCode::kReadFailed;
  }

  // Determine how many bytes are currently available.

  const size_t available_bytes =
      static_cast<size_t>(communication_status.cbInQue);

  const size_t bytes_to_read =
      std::min(max_bytes, available_bytes);

  if (bytes_to_read == 0U) {
    return utils::ErrorCode::kOk;
  }

  buffer.resize(bytes_to_read);

  DWORD bytes_read = 0U;

  if (ReadFile(
          handle,
          buffer.data(),
          static_cast<DWORD>(bytes_to_read),
          &bytes_read,
          nullptr) == FALSE) {

    buffer.clear();

    return utils::ErrorCode::kReadFailed;
  }

  buffer.resize(static_cast<size_t>(bytes_read));

  return utils::ErrorCode::kOk;

#else

  return utils::ErrorCode::kNotInitialized;

#endif
}

}  // namespace stfuzz::adaptor
