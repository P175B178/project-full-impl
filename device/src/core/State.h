#pragma once

#include <string_view>

namespace warden {

/// Normal, Warning and Alert follow the thresholds; Fault means the sensor no longer gives valid
/// readings.
enum class State { Normal, Warning, Alert, Fault };

/// Name of the state, for logs.
[[nodiscard]] constexpr std::string_view toString(State state) noexcept {
  switch (state) {
  case State::Normal:
    return "Normal";
  case State::Warning:
    return "Warning";
  case State::Alert:
    return "Alert";
  case State::Fault:
    return "Fault";
  }
  return "Unknown";
}

} // namespace warden
