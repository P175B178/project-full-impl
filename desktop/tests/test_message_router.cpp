#include "ProtocolExamples.h"
#include "messages/MessageRouter.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QString>

using namespace warden::messages;
using namespace warden::test;

using message_router::route;

TEST(MessageRouterTest, ATemperatureTopicGivesAReadingWithItsGroup) {
  const auto reading = route("sensors/group-1/temperature", protocolExample("observation-temperature.json"));

  ASSERT_TRUE(reading.has_value()) << reading.error().toStdString();
  EXPECT_EQ(reading->groupId, "group-1");
  EXPECT_EQ(reading->reading.metric, observation::Metric::Temperature);
}

TEST(MessageRouterTest, AHumidityTopicGivesAReading) {
  const auto reading = route("sensors/group-1/humidity", protocolExample("observation-humidity.json"));

  ASSERT_TRUE(reading.has_value()) << reading.error().toStdString();
  EXPECT_EQ(reading->reading.metric, observation::Metric::Humidity);
}

TEST(MessageRouterTest, AReadingOnTheOtherMetricsTopicIsDropped) {
  const auto reading = route("sensors/group-1/humidity", protocolExample("observation-temperature.json"));

  ASSERT_FALSE(reading.has_value());
  EXPECT_TRUE(reading.error().contains("its code does not match the topic"));
}

TEST(MessageRouterTest, AMalformedPayloadIsDroppedWithTheParsersReason) {
  const auto reading = route("sensors/group-1/temperature", "not json");

  ASSERT_FALSE(reading.has_value());
  EXPECT_TRUE(reading.error().startsWith("Dropped reading on sensors/group-1/temperature: "));
}

TEST(MessageRouterTest, UnexpectedTopicsAreIgnored) {
  const QByteArray payload = protocolExample("observation-temperature.json");

  for (const char *topic : {"sensors/group-1", "sensors/group-1/temperature/extra", "sensors/group-1/pressure",
                            "other/group-1/temperature", ""}) {
    const auto reading = route(topic, payload);
    ASSERT_FALSE(reading.has_value()) << topic;
    EXPECT_TRUE(reading.error().startsWith("Ignored message on unexpected topic")) << topic;
  }
}
