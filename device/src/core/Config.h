#pragma once

#include "Range.h"
#include "Thresholds.h"

#include <chrono>
#include <cstddef>
#include <expected>
#include <string>
#include <string_view>

namespace warden {

// NOLINTBEGIN(readability-magic-numbers)

/// Values plausible for the site: readings outside them are skipped before averaging. Can be
/// narrower than the sensor's own range: 70 C is a valid DHT22 reading, but not for a room.
struct PlausibleValues {
  Range temperature{.min = 0.0F, .max = 50.0F};
  Range humidity{.min = 0.0F, .max = 100.0F};
};

/// All runtime configuration: mirrors config/warden.toml, one struct per [section].
struct Config {
  std::chrono::seconds readInterval{5};
  std::size_t averagingWindow{10};
  Thresholds thresholds;           ///< [thresholds]
  PlausibleValues plausibleValues; ///< [plausible_values]
};

// NOLINTEND(readability-magic-numbers)

/// Loads a TOML config file; missing settings keep the defaults above. Returns an error message for
/// an unreadable file or a value outside its limits.
[[nodiscard]] std::expected<Config, std::string> loadConfig(std::string_view path);

} // namespace warden
