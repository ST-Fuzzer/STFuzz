#pragma once

#include <array>
#include <string>
#include <string_view>
#include <utility>

#include "utils/ErrorCode.hpp"

namespace stfuzz {
namespace utils {

class SerDes {
 public:
  template <typename Table, typename... Args>
  [[nodiscard]] static stfuzz::utils::ErrorCode Serialize(std::string& out,
                                                          const Table& table,
                                                          std::string_view key,
                                                          const Args&... args) {
    std::string_view fmt;
    if (const auto status = Find(fmt, table, key);
        status != stfuzz::utils::ErrorCode::kOk) {
      return status;
    }

    const std::array<std::string_view, sizeof...(Args)> params{
        std::string_view(args)...};
    std::string text;
    size_t next = 0;

    while (true) {
      const size_t open = fmt.find('<');
      text += fmt.substr(0, open);
      if (open == std::string_view::npos) {
        break;
      }

      const size_t close = fmt.find('>', open);
      if (close == std::string_view::npos) {
        return stfuzz::utils::ErrorCode::kInvalidFormat;
      }
      if (next >= params.size()) {
        return stfuzz::utils::ErrorCode::kMissingArgument;
      }

      text += params[next++];
      fmt.remove_prefix(close + 1);
    }

    if (next != params.size()) {
      return stfuzz::utils::ErrorCode::kUnusedArgument;
    }

    out = std::move(text);
    return stfuzz::utils::ErrorCode::kOk;
  }

  template <typename Table, typename... Outs>
  [[nodiscard]] static stfuzz::utils::ErrorCode Deserialize(
      const Table& table, std::string_view key, std::string_view text,
      Outs&... outs) {
    std::string_view fmt;
    if (const auto status = Find(fmt, table, key);
        status != stfuzz::utils::ErrorCode::kOk) {
      return status;
    }

    const std::array<std::string*, sizeof...(Outs)> slots{&outs...};
    size_t next = 0;

    while (true) {
      const size_t open = fmt.find('<');
      const std::string_view literal = fmt.substr(0, open);

      if (text.substr(0, literal.size()) != literal) {
        return stfuzz::utils::ErrorCode::kMismatch;
      }
      text.remove_prefix(literal.size());
      if (open == std::string_view::npos) {
        break;
      }

      const size_t close = fmt.find('>', open);
      if (close == std::string_view::npos) {
        return stfuzz::utils::ErrorCode::kInvalidFormat;
      }
      fmt.remove_prefix(close + 1);
      if (next >= slots.size()) {
        return stfuzz::utils::ErrorCode::kMissingArgument;
      }

      const std::string_view following = fmt.substr(0, fmt.find('<'));
      size_t end = text.size();
      if (!following.empty()) {
        end = text.find(following);
        if (end == std::string_view::npos) {
          return stfuzz::utils::ErrorCode::kMismatch;
        }
      } else if (!fmt.empty()) {
        return stfuzz::utils::ErrorCode::kInvalidFormat;
      }

      slots[next++]->assign(text.substr(0, end));
      text.remove_prefix(end);
    }

    if (!text.empty()) {
      return stfuzz::utils::ErrorCode::kMismatch;
    }
    if (next != slots.size()) {
      return stfuzz::utils::ErrorCode::kUnusedArgument;
    }
    return stfuzz::utils::ErrorCode::kOk;
  }

 private:
  template <typename Table>
  [[nodiscard]] static stfuzz::utils::ErrorCode Find(std::string_view& fmt,
                                                     const Table& table,
                                                     std::string_view key) {
    for (const auto& entry : table) {
      if (entry.key == key) {
        fmt = entry.value;
        return stfuzz::utils::ErrorCode::kOk;
      }
    }
    return stfuzz::utils::ErrorCode::kKeyNotFound;
  }
};

}  // namespace utils
}  // namespace stfuzz
