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
  const auto message = route("sensors/group-1/status", protocolExample("status-normal.json"));

  ASSERT_TRUE(message.has_value()) << message.error().toStdString();
  const auto *status = std::get_if<message_router::Status>(&message.value());
  ASSERT_NE(status, nullptr);
  EXPECT_EQ(status->groupId, "group-1");
  EXPECT_EQ(status->status.state, status_message::State::Normal);
}

TEST(MessageRouterTest, AMalformedPayloadIsDroppedWithTheParsersReason) {
  const auto reading = route("sensors/group-1/temperature", "not json");

  ASSERT_FALSE(reading.has_value());
  EXPECT_TRUE(reading.error().startsWith("Dropped reading on sensors/group-1/temperature: "));
}

TEST(MessageRouterTest, UnexpectedTopicsAreIgnored) {
  const QByteArray payload = protocolExample("status-normal.json");

  for (const char *topic :
       {"sensors/group-1", "sensors/group-1/status/extra", "sensors/group-1/pressure", "other/group-1/status", ""}) {
    const auto message = route(topic, payload);
    ASSERT_FALSE(message.has_value()) << topic;
    EXPECT_TRUE(message.error().startsWith("Ignored message on unexpected topic")) << topic;
  }
}
