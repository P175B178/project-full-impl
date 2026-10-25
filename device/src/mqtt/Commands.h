#pragma once

#include <chrono>
#include <expected>
#include <functional>
#include <string>
#include <string_view>

namespace warden::mqtt {

/// Parses a temperature threshold command, {"value": 28.0, "unit": "°C"} (see
/// protocol/examples/config-temperature-threshold.json): the value, or what is wrong with it. Both
/// fields are required; unknown ones are ignored. The caller checks the limits.
[[nodiscard]] std::expected<float, std::string> parseTemperatureThreshold(std::string_view payload);

/// The same for humidity, {"value": 50.0, "unit": "%"} (see
/// protocol/examples/config-humidity-threshold.json).
[[nodiscard]] std::expected<float, std::string> parseHumidityThreshold(std::string_view payload);

/// Parses an interval command, {"seconds": 5} (see protocol/examples/config-interval.json): a whole
/// number of seconds, required; unknown fields are ignored. The caller checks the limits.
[[nodiscard]] std::expected<std::chrono::seconds, std::string> parseInterval(std::string_view payload);

/// What to do with each valid command. Called on the MQTT client's thread.
struct CommandHandlers {
  std::function<void(float threshold)> temperatureThreshold;
  std::function<void(float threshold)> humidityThreshold;
  std::function<void(std::chrono::seconds interval)> interval;
};

} // namespace warden::mqtt
