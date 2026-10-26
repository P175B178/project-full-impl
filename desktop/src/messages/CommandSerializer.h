#pragma once

#include "messages/StatusMessage.h"

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

/// `config/<group-id>/state` — body `{"state": "<normal|warning|alert|fault>"}`. The device stays
/// in that state until its next valid reading.
[[nodiscard]] QByteArray forcedState(status_message::State state);
} // namespace warden::messages::command_serializer
