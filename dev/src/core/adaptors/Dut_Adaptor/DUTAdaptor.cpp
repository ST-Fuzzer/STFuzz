#include "core/adaptors/dut_adaptor/DUTAdaptor.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <utility>

#include "core/adaptors/FeedbackSubscriber.hpp"
#include "core/adaptors/StatusSubscriber.hpp"
#include "core/adaptors/dut_adaptor/DUTMessage.hpp"
#include "utils/ErrorCode.hpp"

namespace stfuzz::adaptor {

// IAdaptor

DUTAdaptor::DUTAdaptor(SerialPortSettings serial_settings)
    : serial_link_(std::move(serial_settings)) {}

utils::ErrorCode DUTAdaptor::Connect() {
  const auto result = config_.Validate();

  if (result != utils::ErrorCode::kOk) {
    return result;
  }

  return serial_link_.Open();
}

bool DUTAdaptor::IsConnected() const { return serial_link_.IsOpen(); }

utils::ErrorCode DUTAdaptor::Reset() {
  if (!IsConnected()) {
    return utils::ErrorCode::kNotConnected;
  }

  return utils::ErrorCode::kResetFailed;
}

// IConfig

utils::ErrorCode DUTAdaptor::SetParameter(const std::string& key,
                                          const std::string& value) {
  return config_.SetParameter(key, value);
}

std::optional<std::string> DUTAdaptor::GetValue(const std::string& key) const {
  return config_.GetValue(key);
}

bool DUTAdaptor::HasKey(const std::string& key) const {
  return config_.HasKey(key);
}

utils::ErrorCode DUTAdaptor::Validate() const { return config_.Validate(); }

// ISerial

utils::ErrorCode DUTAdaptor::Open() { return serial_link_.Open(); }

void DUTAdaptor::Close() { serial_link_.Close(); }

bool DUTAdaptor::IsOpen() const { return serial_link_.IsOpen(); }

utils::ErrorCode DUTAdaptor::Write(const std::vector<uint8_t>& data) {
  return serial_link_.Write(data);
}

utils::ErrorCode DUTAdaptor::Read(std::vector<uint8_t>& buffer,
                                  size_t max_bytes) {
  std::vector<uint8_t> received_data;

  const auto result = serial_link_.Read(received_data, max_bytes);

  if (result != utils::ErrorCode::kOk) {
    return result;
  }

  buffer = received_data;

  std::vector<DUTMessage> messages;

  const auto parser_result =
      feedback_parser_.ProcessData(received_data, messages);

  if (parser_result != utils::ErrorCode::kOk) {
    return parser_result;
  }

  for (const auto& message : messages) {
    const auto manager_result = status_manager_.HandleMessage(message);

    if (manager_result != utils::ErrorCode::kOk) {
      return manager_result;
    }
  }

  return utils::ErrorCode::kOk;
}

// IStatus

utils::ErrorCode DUTAdaptor::Subscribe(IStatusSubscriber* subscriber) {
  return status_manager_.Subscribe(subscriber);
}

void DUTAdaptor::Unsubscribe(IStatusSubscriber* subscriber) {
  status_manager_.Unsubscribe(subscriber);
}

// IFeedback

utils::ErrorCode DUTAdaptor::Subscribe(IFeedbackSubscriber* subscriber) {
  return status_manager_.Subscribe(subscriber);
}

void DUTAdaptor::Unsubscribe(IFeedbackSubscriber* subscriber) {
  status_manager_.Unsubscribe(subscriber);
}

}  // namespace stfuzz::adaptor
