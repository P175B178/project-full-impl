#include "Payloads.h"

#include <nlohmann/json.hpp>

#include <cmath>
#include <format>

namespace warden::mqtt {

namespace {

/// UTC, to the second, with the Z the protocol requires: 2026-01-02T03:04:05Z.
std::string isoTimestamp(std::chrono::system_clock::time_point time) {
  return std::format("{:%Y-%m-%dT%H:%M:%SZ}", std::chrono::floor<std::chrono::seconds>(time));
}

/// A float as JSON shows it with every digit of the nearest double: 23.4F would be
/// 23.399999618530273. One decimal is what the sensor resolves.
double oneDecimal(float value) {
  constexpr double decimals = 10.0;
  return std::round(static_cast<double>(value) * decimals) / decimals;
}

} // namespace

std::string temperatureObservation(std::string_view deviceId, float value, std::chrono::system_clock::time_point time) {
  const nlohmann::json observation{
      {"resourceType", "Observation"},
      {"deviceId", deviceId},
      {"status", "final"},
      {"effectiveDateTime", isoTimestamp(time)},
      {"code", {{"system", "http://loinc.org"}, {"code", "8310-5"}, {"display", "Body temperature"}}},
      {"valueQuantity",
       {{"value", oneDecimal(value)}, {"unit", "°C"}, {"system", "http://unitsofmeasure.org"}, {"code", "Cel"}}},
  };
  return observation.dump();
}

std::string humidityObservation(std::string_view deviceId, float value, std::chrono::system_clock::time_point time) {
  const nlohmann::json observation{
      {"resourceType", "Observation"},
      {"deviceId", deviceId},
      {"status", "final"},
      {"effectiveDateTime", isoTimestamp(time)},
      {"code", {{"system", "http://loinc.org"}, {"code", "13834-7"}, {"display", "Humidity"}}},
      {"valueQuantity",
       {{"value", oneDecimal(value)}, {"unit", "%"}, {"system", "http://unitsofmeasure.org"}, {"code", "%"}}},
  };
  return observation.dump();
}

} // namespace warden::mqtt
