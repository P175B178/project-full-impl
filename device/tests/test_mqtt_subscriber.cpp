#include "MqttSubscriber.h"

#include "mocks/MockMqttClient.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace warden::mqtt;
using namespace warden::test;
using ::testing::StrictMock;

/// Subscribes through a mock client and keeps the handlers it was given, by topic, so a test can
/// deliver messages to them. All public: the fixture is plain test data.
class MqttSubscriberTest : public ::testing::Test {
public:
  void SetUp() override {
    EXPECT_CALL(client, subscribe)
        .Times(2)
        .WillRepeatedly(
            [this](const std::string &topic, MqttMessageHandler handler) { handlers[topic] = std::move(handler); });
    MqttSubscriber subscriber{client, "group-7"};
    subscriber.subscribeToCommands(
        {.temperatureThreshold = [this](float threshold) { temperatureThresholds.push_back(threshold); },
         .humidityThreshold    = [this](float threshold) { humidityThresholds.push_back(threshold); }});
  }

  StrictMock<MockMqttClient> client;
  std::map<std::string, MqttMessageHandler> handlers;
  std::vector<float> temperatureThresholds;
  std::vector<float> humidityThresholds;
};

TEST_F(MqttSubscriberTest, SubscribesToTheGroupsThresholdTopics) {
  EXPECT_TRUE(handlers.contains("config/group-7/temperature-threshold"));
  EXPECT_TRUE(handlers.contains("config/group-7/humidity-threshold"));
}

TEST_F(MqttSubscriberTest, HandsAValidThresholdOn) {
  handlers["config/group-7/humidity-threshold"](R"({"value": 60, "unit": "%"})");
  EXPECT_TRUE(temperatureThresholds.empty());
  ASSERT_EQ(humidityThresholds.size(), 1U);
  EXPECT_FLOAT_EQ(humidityThresholds[0], 60.0F);
}

TEST_F(MqttSubscriberTest, DropsAnInvalidMessage) {
  handlers["config/group-7/temperature-threshold"]("not json");
  handlers["config/group-7/temperature-threshold"](R"({"value": 28, "unit": "%"})");
  EXPECT_TRUE(temperatureThresholds.empty());
}

TEST_F(MqttSubscriberTest, TakesAnEmptyMessageAsAClearedCommand) {
  handlers["config/group-7/temperature-threshold"]("");
  EXPECT_TRUE(temperatureThresholds.empty());
}
