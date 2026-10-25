#pragma once

#include "Thresholds.h"

#include <chrono>
#include <cstddef>
#include <expected>
#include <string>
#include <string_view>

namespace warden {

// NOLINTBEGIN(readability-magic-numbers)

/// All runtime configuration: mirrors config/warden.toml, one struct per [section].
struct Config {
  std::chrono::seconds readInterval{5};
  std::size_t averagingWindow{10};
  Thresholds thresholds; ///< [thresholds]
};

// NOLINTEND(readability-magic-numbers)

/// Loads a TOML config file; missing settings keep the defaults above. Returns an error message for
/// an unreadable file or a value outside its limits.
[[nodiscard]] std::expected<Config, std::string> loadConfig(std::string_view path);

} // namespace warden
