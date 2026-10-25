#include "Config.h"

#include <toml++/toml.hpp>

#include <format>

namespace warden {

namespace {

// The upper limits catch typos.
constexpr std::size_t maxAveragingWindow       = 1000;
constexpr std::size_t maxFaultAfterFailedReads = 100;
constexpr float maxHumidity                    = 100.0F; // relative humidity is a percentage
constexpr int maxPort                          = 65535;

/// Whether `level` can be one level of an MQTT topic: + and # are wildcards, and / separates
/// levels.
bool isTopicLevel(std::string_view level) {
  return !level.empty() && level.find_first_of("/+#") == std::string_view::npos;
}

} // namespace

std::expected<Config, std::string> loadConfig(std::string_view path) {
  try {
    const toml::table table = toml::parse_file(path);

    const Config defaults{};
    // Not const, so the return below can move it instead of copying.
    Config config{
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
        .mqtt =
            {
                .host     = table.at_path("mqtt.host").value_or(defaults.mqtt.host),
                .port     = table.at_path("mqtt.port").value_or(defaults.mqtt.port),
                .groupId  = table.at_path("mqtt.group_id").value_or(defaults.mqtt.groupId),
                .deviceId = table.at_path("mqtt.device_id").value_or(defaults.mqtt.deviceId),
            },
    };

    if (!isUsableReadInterval(config.readInterval)) {
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

    if (config.mqtt.groupId.empty()) {
      return std::unexpected{"mqtt.group_id must be set: your group's id, such as group-1"};
    }
    if (!isTopicLevel(config.mqtt.groupId)) {
      return std::unexpected{"mqtt.group_id must not contain /, + or #: it is part of every topic"};
    }

    if (config.mqtt.host.empty()) {
      return std::unexpected{"mqtt.host must be set"};
    }
    if (config.mqtt.port < 1 || config.mqtt.port > maxPort) {
      return std::unexpected{std::format("mqtt.port must be between 1 and {}", maxPort)};
    }

    return config;
  } catch (const toml::parse_error &err) {
    return std::unexpected{std::string{err.description()}};
  }
}

} // namespace warden
