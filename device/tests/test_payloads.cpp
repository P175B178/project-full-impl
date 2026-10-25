#include "Payloads.h"

#include "ProtocolExamples.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <chrono>

using namespace warden::mqtt;
using warden::test::protocolExample;

namespace {
// The time in the examples in protocol/examples: 2026-01-02T03:04:05Z.
const std::chrono::system_clock::time_point exampleTime = std::chrono::sys_days{std::chrono::January / 2 / 2026} +
                                                          std::chrono::hours{3} + std::chrono::minutes{4} +
                                                          std::chrono::seconds{5};
} // namespace

// The device's messages must be exactly the protocol's examples, field for field; then the desktop
// program, tested against the same files, reads them.

TEST(PayloadsTest, TemperatureObservationMatchesTheProtocol) {
  const auto payload = temperatureObservation("group-1-pi5", 23.4F, exampleTime);
  EXPECT_EQ(nlohmann::json::parse(payload), protocolExample("observation-temperature.json"));
}

TEST(PayloadsTest, HumidityObservationMatchesTheProtocol) {
  const auto payload = humidityObservation("group-1-pi5", 45.0F, exampleTime);
  EXPECT_EQ(nlohmann::json::parse(payload), protocolExample("observation-humidity.json"));
}

TEST(PayloadsTest, ValueIsRoundedToOneDecimal) {
  const auto payload = nlohmann::json::parse(temperatureObservation("d", 23.46F, exampleTime));
  EXPECT_DOUBLE_EQ(payload["valueQuantity"]["value"].get<double>(), 23.5);
}

TEST(PayloadsTest, TimeIsUtcToTheSecond) {
  const auto payload =
      nlohmann::json::parse(humidityObservation("d", 50.0F, exampleTime + std::chrono::milliseconds{999}));
  EXPECT_EQ(payload["effectiveDateTime"], "2026-01-02T03:04:05Z");
}
