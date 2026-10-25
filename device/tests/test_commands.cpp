#include "Commands.h"

#include "ProtocolExamples.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <string>

using namespace warden;
using namespace warden::mqtt;
using namespace warden::test;
using ::testing::HasSubstr;

// ── Threshold commands ──────────────────────────────────────────────────────

TEST(ParseThresholdTest, ReadsTheProtocolsTemperatureExample) {
  const auto threshold = parseTemperatureThreshold(protocolExample("config-temperature-threshold.json").dump());
  ASSERT_TRUE(threshold.has_value()) << threshold.error();
  EXPECT_FLOAT_EQ(*threshold, 28.0F);
}

TEST(ParseThresholdTest, ReadsTheProtocolsHumidityExample) {
  const auto threshold = parseHumidityThreshold(protocolExample("config-humidity-threshold.json").dump());
  ASSERT_TRUE(threshold.has_value()) << threshold.error();
  EXPECT_FLOAT_EQ(*threshold, 50.0F);
}

TEST(ParseThresholdTest, IgnoresUnknownFields) {
  const auto threshold = parseTemperatureThreshold(R"({"value": 30, "unit": "°C", "sentBy": "desktop"})");
  ASSERT_TRUE(threshold.has_value()) << threshold.error();
  EXPECT_FLOAT_EQ(*threshold, 30.0F);
}

TEST(ParseThresholdTest, RejectsWhatIsNotJson) {
  EXPECT_THAT(parseTemperatureThreshold("28").error(), HasSubstr("not a JSON object"));
  EXPECT_THAT(parseTemperatureThreshold("{value: 28}").error(), HasSubstr("not valid JSON"));
  EXPECT_THAT(parseTemperatureThreshold("").error(), HasSubstr("not valid JSON"));
}

TEST(ParseThresholdTest, RejectsAMissingOrNonNumericValue) {
  EXPECT_THAT(parseTemperatureThreshold(R"({"unit": "°C"})").error(), HasSubstr(R"("value" must be a number)"));
  EXPECT_THAT(parseTemperatureThreshold(R"({"value": "28", "unit": "°C"})").error(),
              HasSubstr(R"("value" must be a number)"));
}

TEST(ParseThresholdTest, RejectsAMissingOrWrongUnit) {
  EXPECT_THAT(parseTemperatureThreshold(R"({"value": 28})").error(), HasSubstr(R"("unit" must be a string)"));
  EXPECT_THAT(parseTemperatureThreshold(R"({"value": 28, "unit": "%"})").error(), HasSubstr(R"("unit" must be "°C")"));
  EXPECT_THAT(parseHumidityThreshold(R"({"value": 50, "unit": "°C"})").error(), HasSubstr(R"("unit" must be "%")"));
}

TEST(ParseThresholdTest, RejectsAValueNoFloatHolds) {
  EXPECT_THAT(parseTemperatureThreshold(R"({"value": 1e300, "unit": "°C"})").error(), HasSubstr("out of range"));
}

// ── parseInterval ───────────────────────────────────────────────────────────

TEST(ParseIntervalTest, ReadsTheProtocolsExample) {
  const auto interval = parseInterval(protocolExample("config-interval.json").dump());
  ASSERT_TRUE(interval.has_value()) << interval.error();
  EXPECT_EQ(*interval, std::chrono::seconds{5});
}

TEST(ParseIntervalTest, ReadsAWholeNumberWrittenWithADecimalPoint) {
  EXPECT_EQ(parseInterval(R"({"seconds": 5.0})").value(), std::chrono::seconds{5});
}

TEST(ParseIntervalTest, RejectsANumberTooLargeForSeconds) {
  EXPECT_THAT(parseInterval(R"({"seconds": 1e300})").error(), HasSubstr(R"("seconds" must be a whole number)"));
}

TEST(ParseIntervalTest, RejectsAMissingOrFractionalValue) {
  EXPECT_THAT(parseInterval("{}").error(), HasSubstr(R"("seconds" must be a whole number)"));
  EXPECT_THAT(parseInterval(R"({"seconds": 2.5})").error(), HasSubstr(R"("seconds" must be a whole number)"));
  EXPECT_THAT(parseInterval(R"({"seconds": "5"})").error(), HasSubstr(R"("seconds" must be a whole number)"));
}

TEST(ParseIntervalTest, RejectsWhatIsNotAJsonObject) {
  EXPECT_THAT(parseInterval("5").error(), HasSubstr("not a JSON object"));
  EXPECT_THAT(parseInterval("{").error(), HasSubstr("not valid JSON"));
}

// ── parseForcedState ────────────────────────────────────────────────────────

TEST(ParseForcedStateTest, ReadsTheProtocolsExample) {
  const auto state = parseForcedState(protocolExample("config-state.json").dump());
  ASSERT_TRUE(state.has_value()) << state.error();
  EXPECT_EQ(*state, State::Alert);
}

TEST(ParseForcedStateTest, ReadsEveryState) {
  EXPECT_EQ(parseForcedState(R"({"state": "normal"})").value(), State::Normal);
  EXPECT_EQ(parseForcedState(R"({"state": "warning"})").value(), State::Warning);
  EXPECT_EQ(parseForcedState(R"({"state": "fault"})").value(), State::Fault);
}

TEST(ParseForcedStateTest, RejectsAnUnknownOrMissingState) {
  EXPECT_THAT(parseForcedState(R"({"state": "offline"})").error(), HasSubstr(R"(not "offline")"));
  EXPECT_THAT(parseForcedState(R"({"color": "red"})").error(), HasSubstr(R"("state" must be a string)"));
  EXPECT_THAT(parseForcedState("alert").error(), HasSubstr("not valid JSON"));
}
