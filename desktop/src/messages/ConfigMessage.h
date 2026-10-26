#pragma once

#include <QByteArray>
#include <QString>

#include <expected>

/// Parses the retained commands on `config/<group-id>/...` (see protocol/examples/config-*.json):
/// the settings a device was last sent, by any console. Malformed input, or a value the device
/// would reject, gives an error string.
namespace warden::messages::config_message {

// The device's limits (device/config/warden.toml); for temperature, its default plausible range.
constexpr double minTemperatureThreshold = 0.0;
constexpr double maxTemperatureThreshold = 50.0;
constexpr double minHumidityThreshold    = 0.0;
constexpr double maxHumidityThreshold    = 100.0;
constexpr int minInterval                = 2;
constexpr int maxInterval                = 3600;

/// `config/<group-id>/temperature-threshold` — returns the threshold in °C.
[[nodiscard]] std::expected<double, QString> parseTemperatureThreshold(const QByteArray &payload);

/// `config/<group-id>/humidity-threshold` — returns the threshold in %.
[[nodiscard]] std::expected<double, QString> parseHumidityThreshold(const QByteArray &payload);

/// `config/<group-id>/interval` — returns the interval in seconds.
[[nodiscard]] std::expected<int, QString> parseInterval(const QByteArray &payload);
} // namespace warden::messages::config_message
