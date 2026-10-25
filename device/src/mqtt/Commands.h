#pragma once

#include <expected>
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

} // namespace warden::mqtt
