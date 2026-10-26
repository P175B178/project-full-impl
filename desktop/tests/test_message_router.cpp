#include "ProtocolExamples.h"
#include "messages/MessageRouter.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QString>

#include <variant>

using namespace warden::messages;
using namespace warden::test;

using message_router::route;

TEST(MessageRouterTest, ATemperatureTopicGivesAReadingWithItsGroup) {
  const auto message = route("sensors/group-1/temperature", protocolExample("observation-temperature.json"));

  ASSERT_TRUE(message.has_value()) << message.error().toStdString();
  const auto *reading = std::get_if<message_router::Reading>(&message.value());
  ASSERT_NE(reading, nullptr);
  EXPECT_EQ(reading->groupId, "group-1");
  EXPECT_EQ(reading->reading.metric, observation::Metric::Temperature);
}

TEST(MessageRouterTest, AHumidityTopicGivesAReading) {
  const auto message = route("sensors/group-1/humidity", protocolExample("observation-humidity.json"));

  ASSERT_TRUE(message.has_value()) << message.error().toStdString();
  const auto *reading = std::get_if<message_router::Reading>(&message.value());
  ASSERT_NE(reading, nullptr);
  EXPECT_EQ(reading->reading.metric, observation::Metric::Humidity);
}

TEST(MessageRouterTest, AReadingOnTheOtherMetricsTopicIsDropped) {
  const auto message = route("sensors/group-1/humidity", protocolExample("observation-temperature.json"));

  ASSERT_FALSE(message.has_value());
  EXPECT_TRUE(message.error().contains("its code does not match the topic"));
}

TEST(MessageRouterTest, AStatusTopicGivesAStatus) {
  const auto message = route("sensors/group-2/status", protocolExample("status-fault.json"));

  ASSERT_TRUE(message.has_value()) << message.error().toStdString();
  const auto *status = std::get_if<message_router::Status>(&message.value());
  ASSERT_NE(status, nullptr);
  EXPECT_EQ(status->groupId, "group-2");
  EXPECT_EQ(status->status.state, status_message::State::Fault);
}

TEST(MessageRouterTest, ConfigTopicsGiveTheSettings) {
  const auto temperature =
      route("config/group-1/temperature-threshold", protocolExample("config-temperature-threshold.json"));
  const auto humidity = route("config/group-1/humidity-threshold", protocolExample("config-humidity-threshold.json"));
  const auto interval = route("config/group-1/interval", protocolExample("config-interval.json"));

  ASSERT_TRUE(temperature.has_value() && humidity.has_value() && interval.has_value());
  EXPECT_EQ(std::get<message_router::TemperatureThreshold>(temperature.value()).celsius, 28.0);
  EXPECT_EQ(std::get<message_router::HumidityThreshold>(humidity.value()).percent, 50.0);
  EXPECT_EQ(std::get<message_router::Interval>(interval.value()).seconds, 5);
  EXPECT_EQ(std::get<message_router::Interval>(interval.value()).groupId, "group-1");
}

TEST(MessageRouterTest, AnEmptySettingIsAClearedOne) {
  const auto message = route("config/group-1/interval", QByteArray());

  ASSERT_TRUE(message.has_value()) << message.error().toStdString();
  EXPECT_FALSE(std::get<message_router::Interval>(message.value()).seconds.has_value());
}

TEST(MessageRouterTest, AMalformedPayloadIsDroppedWithTheParsersReason) {
  const auto reading = route("sensors/group-1/temperature", "not json");
  const auto setting = route("config/group-1/interval", R"({"seconds": "five"})");

  ASSERT_FALSE(reading.has_value());
  EXPECT_TRUE(reading.error().startsWith("Dropped reading on sensors/group-1/temperature: "));
  ASSERT_FALSE(setting.has_value());
  EXPECT_TRUE(setting.error().startsWith("Dropped setting on config/group-1/interval: "));
}

TEST(MessageRouterTest, UnexpectedTopicsAreIgnored) {
  const QByteArray payload = protocolExample("status-normal.json");

  for (const char *topic : {"sensors/group-1", "sensors/group-1/status/extra", "sensors/group-1/pressure",
                            "config/group-1/state", "other/group-1/status", ""}) {
    const auto message = route(topic, payload);
    ASSERT_FALSE(message.has_value()) << topic;
    EXPECT_TRUE(message.error().startsWith("Ignored message on unexpected topic")) << topic;
  }
}
