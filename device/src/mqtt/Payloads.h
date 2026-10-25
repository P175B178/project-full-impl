#pragma once

#include "State.h"

#include <chrono>
#include <string>
#include <string_view>

namespace warden::mqtt {

/// The JSON message for a temperature reading: an HL7 FHIR Observation, as specified in protocol/
/// (see protocol/examples/observation-temperature.json). The value is rounded to one decimal, the
/// sensor's resolution, and the time is UTC, to the second.
[[nodiscard]] std::string
temperatureObservation(std::string_view deviceId, float value, std::chrono::system_clock::time_point time);

/// The same for humidity (see protocol/examples/observation-humidity.json).
[[nodiscard]] std::string
humidityObservation(std::string_view deviceId, float value, std::chrono::system_clock::time_point time);

/// The JSON message for the device's state: {"deviceId", "state", "timestamp"}, as specified in
/// protocol/ (see protocol/examples/status-*.json).
[[nodiscard]] std::string
statusPayload(std::string_view deviceId, State state, std::chrono::system_clock::time_point time);

/// The status message for a device that is gone: state "offline" (see
/// protocol/examples/status-offline.json). Not a State of its own: the device is never in it,
/// others only see it in its place.
[[nodiscard]] std::string offlinePayload(std::string_view deviceId, std::chrono::system_clock::time_point time);

} // namespace warden::mqtt
