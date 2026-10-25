#include "Commands.h"

#include <nlohmann/json.hpp>

#include <cmath>
#include <cstdint>
#include <format>

namespace warden::mqtt {

namespace {

// Far above any usable interval, and exact as a double: a larger number would not fit the cast to a
// whole number of seconds.
constexpr double maxSeconds = 1e9;

/// Parses a command's payload, which must be a JSON object.
std::expected<nlohmann::json, std::string> parseObject(std::string_view payload) {
  nlohmann::json command;
  try {
    command = nlohmann::json::parse(payload);
  } catch (const nlohmann::json::parse_error &) {
    return std::unexpected{"not valid JSON"};
  }
  if (!command.is_object()) {
    return std::unexpected{"not a JSON object"};
  }
  return command;
}

/// Parses a threshold command, whose unit must be `expectedUnit`.
std::expected<float, std::string> parseThreshold(std::string_view payload, const std::string &expectedUnit) {
  const auto parsed = parseObject(payload);
  if (!parsed) {
    return std::unexpected{parsed.error()};
  }
  const auto value = parsed->find("value");
  if (value == parsed->end() || !value->is_number()) {
    return std::unexpected{R"("value" must be a number)"};
  }
  const auto unit = parsed->find("unit");
  if (unit == parsed->end() || !unit->is_string()) {
    return std::unexpected{R"("unit" must be a string)"};
  }
  if (unit->get<std::string>() != expectedUnit) {
    return std::unexpected{std::format(R"("unit" must be "{}")", expectedUnit)};
  }
  // JSON allows numbers far beyond what a float holds; they would become inf.
  const auto threshold = static_cast<float>(value->get<double>());
  if (!std::isfinite(threshold)) {
    return std::unexpected{R"("value" is out of range)"};
  }
  return threshold;
}

} // namespace

std::expected<float, std::string> parseTemperatureThreshold(std::string_view payload) {
  return parseThreshold(payload, "°C");
}

std::expected<float, std::string> parseHumidityThreshold(std::string_view payload) {
  return parseThreshold(payload, "%");
}

std::expected<std::chrono::seconds, std::string> parseInterval(std::string_view payload) {
  const auto parsed = parseObject(payload);
  if (!parsed) {
    return std::unexpected{parsed.error()};
  }
  const auto seconds = parsed->find("seconds");
  if (seconds == parsed->end() || !seconds->is_number()) {
    return std::unexpected{R"("seconds" must be a whole number)"};
  }
  // 5.0 is the same JSON number as 5, and some programs write it so.
  const auto value = seconds->get<double>();
  if (value != std::trunc(value) || std::abs(value) > maxSeconds) {
    return std::unexpected{R"("seconds" must be a whole number)"};
  }
  return std::chrono::seconds{static_cast<std::int64_t>(value)};
}

std::expected<State, std::string> parseForcedState(std::string_view payload) {
  const auto parsed = parseObject(payload);
  if (!parsed) {
    return std::unexpected{parsed.error()};
  }
  const auto state = parsed->find("state");
  if (state == parsed->end() || !state->is_string()) {
    return std::unexpected{R"("state" must be a string)"};
  }
  const auto name = state->get<std::string>();
  if (name == "normal") {
    return State::Normal;
  }
  if (name == "warning") {
    return State::Warning;
  }
  if (name == "alert") {
    return State::Alert;
  }
  if (name == "fault") {
    return State::Fault;
  }
  return std::unexpected{std::format(R"("state" must be normal, warning, alert or fault, not "{}")", name)};
}

} // namespace warden::mqtt
