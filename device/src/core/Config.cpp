#include "Config.h"

#include <toml++/toml.hpp>

#include <format>

namespace warden {

namespace {

// The DHT22 needs 2 s between readings; the upper limits catch typos.
constexpr auto minReadInterval           = std::chrono::seconds{2};
constexpr auto maxReadInterval           = std::chrono::seconds{3600};
constexpr std::size_t maxAveragingWindow = 1000;
constexpr float maxHumidity              = 100.0F; // relative humidity is a percentage

} // namespace

std::expected<Config, std::string> loadConfig(std::string_view path) {
  try {
    const toml::table table = toml::parse_file(path);

    const Config defaults{};
    const Config config{
        .readInterval = std::chrono::seconds{table.at_path("read_interval_s").value_or(defaults.readInterval.count())},
        .averagingWindow = table.at_path("averaging_window").value_or(defaults.averagingWindow),

        .thresholds =
            {
                .temperature = table.at_path("thresholds.temperature").value_or(defaults.thresholds.temperature),
                .humidity    = table.at_path("thresholds.humidity").value_or(defaults.thresholds.humidity),
            },
    };

    if (config.readInterval < minReadInterval || config.readInterval > maxReadInterval) {
      return std::unexpected{
          std::format("read_interval_s must be between {} and {}", minReadInterval.count(), maxReadInterval.count())};
    }
    if (config.averagingWindow == 0 || config.averagingWindow > maxAveragingWindow) {
      return std::unexpected{std::format("averaging_window must be between 1 and {}", maxAveragingWindow)};
    }
    if (config.thresholds.humidity < 0.0F || config.thresholds.humidity > maxHumidity) {
      return std::unexpected{std::format("thresholds.humidity must be between 0 and {}", maxHumidity)};
    }

    return config;
  } catch (const toml::parse_error &err) {
    return std::unexpected{std::string{err.description()}};
  }
}

} // namespace warden
