#include "Config.h"

#include <toml++/toml.hpp>

#include <format>

namespace warden {

namespace {

// The DHT22 needs 2 s between readings; the upper limits catch typos.
constexpr auto minReadInterval                 = std::chrono::seconds{2};
constexpr auto maxReadInterval                 = std::chrono::seconds{3600};
constexpr std::size_t maxAveragingWindow       = 1000;
constexpr std::size_t maxFaultAfterFailedReads = 100;
constexpr float maxHumidity                    = 100.0F; // relative humidity is a percentage

} // namespace

std::expected<Config, std::string> loadConfig(std::string_view path) {
  try {
    const toml::table table = toml::parse_file(path);

    const Config defaults{};
    const Config config{
        .readInterval = std::chrono::seconds{table.at_path("read_interval_s").value_or(defaults.readInterval.count())},
        .averagingWindow       = table.at_path("averaging_window").value_or(defaults.averagingWindow),
        .faultAfterFailedReads = table.at_path("fault_after_failed_reads").value_or(defaults.faultAfterFailedReads),

        .thresholds =
            {
                .temperature = table.at_path("thresholds.temperature").value_or(defaults.thresholds.temperature),
                .humidity    = table.at_path("thresholds.humidity").value_or(defaults.thresholds.humidity),
            },
        .plausibleValues =
            {
                .temperature =
                    {
                        .min = table.at_path("plausible_values.min_temperature")
                                   .value_or(defaults.plausibleValues.temperature.min),
                        .max = table.at_path("plausible_values.max_temperature")
                                   .value_or(defaults.plausibleValues.temperature.max),
                    },
                .humidity =
                    {
                        .min = table.at_path("plausible_values.min_humidity")
                                   .value_or(defaults.plausibleValues.humidity.min),
                        .max = table.at_path("plausible_values.max_humidity")
                                   .value_or(defaults.plausibleValues.humidity.max),
                    },
            },
    };

    if (config.readInterval < minReadInterval || config.readInterval > maxReadInterval) {
      return std::unexpected{
          std::format("read_interval_s must be between {} and {}", minReadInterval.count(), maxReadInterval.count())};
    }
    if (config.averagingWindow == 0 || config.averagingWindow > maxAveragingWindow) {
      return std::unexpected{std::format("averaging_window must be between 1 and {}", maxAveragingWindow)};
    }
    if (config.faultAfterFailedReads == 0 || config.faultAfterFailedReads > maxFaultAfterFailedReads) {
      return std::unexpected{
          std::format("fault_after_failed_reads must be between 1 and {}", maxFaultAfterFailedReads)};
    }
    if (config.thresholds.humidity < 0.0F || config.thresholds.humidity > maxHumidity) {
      return std::unexpected{std::format("thresholds.humidity must be between 0 and {}", maxHumidity)};
    }
    // Written so that nan, which TOML allows, fails these checks too.
    if (!(config.plausibleValues.temperature.min < config.plausibleValues.temperature.max)) {
      return std::unexpected{"plausible_values: min_temperature must be less than max_temperature"};
    }
    if (!(config.plausibleValues.humidity.min < config.plausibleValues.humidity.max)) {
      return std::unexpected{"plausible_values: min_humidity must be less than max_humidity"};
    }
    if (config.plausibleValues.humidity.min < 0.0F || config.plausibleValues.humidity.max > maxHumidity) {
      return std::unexpected{
          std::format("plausible_values: min_humidity and max_humidity must be between 0 and {}", maxHumidity)};
    }
    // Outside the plausible range, a threshold is never or always exceeded. The maximum itself is
    // allowed: it switches that alert off.
    if (!config.plausibleValues.temperature.contains(config.thresholds.temperature)) {
      return std::unexpected{std::format("thresholds.temperature must lie in plausible_values: between {} and {}",
                                         config.plausibleValues.temperature.min,
                                         config.plausibleValues.temperature.max)};
    }
    if (!config.plausibleValues.humidity.contains(config.thresholds.humidity)) {
      return std::unexpected{std::format("thresholds.humidity must lie in plausible_values: between {} and {}",
                                         config.plausibleValues.humidity.min, config.plausibleValues.humidity.max)};
    }

    return config;
  } catch (const toml::parse_error &err) {
    return std::unexpected{std::string{err.description()}};
  }
}

} // namespace warden
