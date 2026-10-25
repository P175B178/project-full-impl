#pragma once

namespace warden {

/// Closed interval [min, max] of accepted values.
struct Range {
  float min{};
  float max{};

  [[nodiscard]] constexpr bool contains(float value) const noexcept { return value >= min && value <= max; }
};

} // namespace warden
