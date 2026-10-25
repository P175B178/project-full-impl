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

/// The broker to connect to, and who this device is on it.
struct MqttSettings {
  /// This computer, as seen from the dev container.
  std::string host{"host.docker.internal"};
  int port{1883}; // the standard MQTT port

  /// The last part of your repository's name (group-1 for cpp-2027p-projektas-group-1). Every topic
  /// starts with it. Required.
  std::string groupId;
  /// Identifies this device in every message. Empty: main() uses <groupId>-pi5 on the Pi and
  /// <groupId>-sim in the simulation, so a message shows which one sent it.
  std::string deviceId;
};

/// The read interval's limits, for the config file and commands alike. The DHT22 needs 2 s between
/// readings; above an hour, a value is more likely a typo.
constexpr std::chrono::seconds minReadInterval{2};
constexpr std::chrono::seconds maxReadInterval{3600};

[[nodiscard]] constexpr bool isUsableReadInterval(std::chrono::seconds interval) noexcept {
  return interval >= minReadInterval && interval <= maxReadInterval;
}

/// All runtime configuration: mirrors config/warden.toml, one struct per [section].
struct Config {
  std::chrono::seconds readInterval{5};
  std::size_t averagingWindow{10};
  std::size_t faultAfterFailedReads{5}; ///< Failed or implausible reads in a row
  Thresholds thresholds;                ///< [thresholds]
  PlausibleValues plausibleValues;      ///< [plausible_values]
  MqttSettings mqtt;                    ///< [mqtt]
};

// NOLINTEND(readability-magic-numbers)

/// Loads a TOML config file; missing settings keep the defaults above, except mqtt.group_id, which
/// must be set. Returns an error message for an unreadable file or a value outside its limits.
[[nodiscard]] std::expected<Config, std::string> loadConfig(std::string_view path);

} // namespace warden
