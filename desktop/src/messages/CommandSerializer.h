#pragma once

#include <QByteArray>

/// Builds the command payloads the console publishes to `config/<group-id>/...` (see
/// protocol/examples/config-*.json).
namespace warden::messages::command_serializer {

/// `config/<group-id>/temperature-threshold` — body `{"value": <c>, "unit": "°C"}`.
[[nodiscard]] QByteArray temperatureThreshold(double celsius);

/// `config/<group-id>/humidity-threshold` — body `{"value": <p>, "unit": "%"}`.
[[nodiscard]] QByteArray humidityThreshold(double percent);

/// `config/<group-id>/interval` — body `{"seconds": <s>}`.
[[nodiscard]] QByteArray interval(int seconds);
} // namespace warden::messages::command_serializer
