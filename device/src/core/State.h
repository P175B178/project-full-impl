#pragma once

#include <string_view>

namespace warden {

enum class State { Normal, Warning, Alert };

/// Name of the state, for logs.
[[nodiscard]] constexpr std::string_view toString(State state) noexcept {
  switch (state) {
  case State::Normal:
    return "Normal";
  case State::Warning:
    return "Warning";
  case State::Alert:
    return "Alert";
  }
  return "Unknown";
}

} // namespace warden
