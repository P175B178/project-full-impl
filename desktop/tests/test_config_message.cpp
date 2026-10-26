#include "messages/ConfigMessage.h"

#include "ProtocolExamples.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace warden::messages;
using namespace warden::test;

namespace {

QByteArray withField(const char *example, const QString &field, const QJsonValue &value) {
  QJsonObject object = protocolExampleObject(example);
  object[field]      = value;
  return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

} // namespace

TEST(ConfigMessageTest, ParsesTheTemperatureThresholdExample) {
  const auto threshold =
      config_message::parseTemperatureThreshold(protocolExample("config-temperature-threshold.json"));

  ASSERT_TRUE(threshold.has_value());
  EXPECT_DOUBLE_EQ(*threshold, 28.0);
}

TEST(ConfigMessageTest, ParsesTheHumidityThresholdExample) {
  const auto threshold = config_message::parseHumidityThreshold(protocolExample("config-humidity-threshold.json"));

  ASSERT_TRUE(threshold.has_value());
  EXPECT_DOUBLE_EQ(*threshold, 50.0);
}

TEST(ConfigMessageTest, ParsesTheIntervalExample) {
  const auto interval = config_message::parseInterval(protocolExample("config-interval.json"));

  ASSERT_TRUE(interval.has_value());
  EXPECT_EQ(*interval, 5);
}

TEST(ConfigMessageTest, IgnoresUnknownFields) {
  EXPECT_TRUE(config_message::parseInterval(withField("config-interval.json", "extra", 1)).has_value());
}

TEST(ConfigMessageTest, RejectsMalformedPayloads) {
  EXPECT_FALSE(config_message::parseInterval("not json").has_value());
  EXPECT_FALSE(config_message::parseInterval("[5]").has_value());
  EXPECT_FALSE(config_message::parseInterval("").has_value());
  EXPECT_FALSE(config_message::parseTemperatureThreshold("not json").has_value());
  EXPECT_FALSE(config_message::parseTemperatureThreshold("").has_value());
}

TEST(ConfigMessageTest, RejectsAThresholdWithoutANumber) {
  EXPECT_FALSE(config_message::parseTemperatureThreshold(
                   withField("config-temperature-threshold.json", "value", QStringLiteral("28")))
                   .has_value());
}

TEST(ConfigMessageTest, RejectsAThresholdInTheWrongUnit) {
  EXPECT_FALSE(config_message::parseTemperatureThreshold(
                   withField("config-temperature-threshold.json", "unit", QStringLiteral("%")))
                   .has_value());
  EXPECT_FALSE(config_message::parseHumidityThreshold(
                   withField("config-humidity-threshold.json", "unit", QString::fromUtf8("°C")))
                   .has_value());
}

TEST(ConfigMessageTest, HoldsThresholdsToTheDeviceLimits) {
  const auto temperature = [](double value) {
    return config_message::parseTemperatureThreshold(withField("config-temperature-threshold.json", "value", value));
  };
  const auto humidity = [](double value) {
    return config_message::parseHumidityThreshold(withField("config-humidity-threshold.json", "value", value));
  };

  EXPECT_TRUE(temperature(config_message::minTemperatureThreshold).has_value());
  EXPECT_TRUE(temperature(config_message::maxTemperatureThreshold).has_value());
  EXPECT_FALSE(temperature(config_message::minTemperatureThreshold - 1).has_value());
  EXPECT_FALSE(temperature(config_message::maxTemperatureThreshold + 1).has_value());
  EXPECT_TRUE(humidity(config_message::maxHumidityThreshold).has_value());
  EXPECT_FALSE(humidity(config_message::maxHumidityThreshold + 1).has_value());
}

TEST(ConfigMessageTest, HoldsTheIntervalToTheDeviceLimits) {
  const auto interval = [](double seconds) {
    return config_message::parseInterval(withField("config-interval.json", "seconds", seconds));
  };

  EXPECT_TRUE(interval(config_message::minInterval).has_value());
  EXPECT_TRUE(interval(config_message::maxInterval).has_value());
  EXPECT_FALSE(interval(config_message::minInterval - 1).has_value());
  EXPECT_FALSE(interval(config_message::maxInterval + 1).has_value());
}

TEST(ConfigMessageTest, RejectsAnIntervalThatIsNotAWholeNumber) {
  EXPECT_FALSE(config_message::parseInterval(withField("config-interval.json", "seconds", 2.5)).has_value());
  EXPECT_FALSE(
      config_message::parseInterval(withField("config-interval.json", "seconds", QStringLiteral("5"))).has_value());
}
