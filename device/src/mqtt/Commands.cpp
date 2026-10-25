#include "Commands.h"

#include <nlohmann/json.hpp>

#include <cmath>
#include <format>

namespace warden::mqtt {

namespace {

/// Parses a threshold command, whose unit must be `expectedUnit`.
std::expected<float, std::string> parseThreshold(std::string_view payload, const std::string &expectedUnit) {
  nlohmann::json command;
  try {
    command = nlohmann::json::parse(payload);
  } catch (const nlohmann::json::parse_error &) {
    return std::unexpected{"not valid JSON"};
  }
  if (!command.is_object()) {
    return std::unexpected{"not a JSON object"};
  }
  const auto value = command.find("value");
  if (value == command.end() || !value->is_number()) {
    return std::unexpected{R"("value" must be a number)"};
  }
  const auto unit = command.find("unit");
  if (unit == command.end() || !unit->is_string()) {
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

} // namespace warden::mqtt
