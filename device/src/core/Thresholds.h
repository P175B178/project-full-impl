#pragma once

namespace warden {

/// Limits above which a value counts as exceeded. A struct, so the two cannot be swapped by
/// mistake.
// NOLINTBEGIN(readability-magic-numbers)
struct Thresholds {
  float temperature{28.0F}; ///< Degrees C
  float humidity{70.0F};    ///< Relative humidity in %
};
// NOLINTEND(readability-magic-numbers)

} // namespace warden
