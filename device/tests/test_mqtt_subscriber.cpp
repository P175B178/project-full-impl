#include "MqttSubscriber.h"

#include "mocks/MockMqttClient.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace warden;
using namespace warden::mqtt;
using namespace warden::test;
using ::testing::StrictMock;

/// Subscribes through a mock client and keeps the handlers it was given, by topic, so a test can
/// deliver messages to them. All public: the fixture is plain test data.
class MqttSubscriberTest : public ::testing::Test {
public:
  void SetUp() override {
    EXPECT_CALL(client, subscribe)
        .Times(4)
        .WillRepeatedly(
            [this](const std::string &topic, MqttMessageHandler handler) { handlers[topic] = std::move(handler); });
    MqttSubscriber subscriber{client, "group-7"};
    subscriber.subscribeToCommands({
        .temperatureThreshold = [this](float threshold) { temperatureThresholds.push_back(threshold); },
        .humidityThreshold    = [this](float threshold) { humidityThresholds.push_back(threshold); },
        .interval             = [this](std::chrono::seconds interval) { intervals.push_back(interval); },
        .state                = [this](State state) { states.push_back(state); },
    });
  }

  StrictMock<MockMqttClient> client;
  std::map<std::string, MqttMessageHandler> handlers;
  std::vector<float> temperatureThresholds;
  std::vector<float> humidityThresholds;
  std::vector<std::chrono::seconds> intervals;
  std::vector<State> states;
};

TEST_F(MqttSubscriberTest, SubscribesToTheGroupsCommandTopics) {
  EXPECT_TRUE(handlers.contains("config/group-7/temperature-threshold"));
  EXPECT_TRUE(handlers.contains("config/group-7/humidity-threshold"));
  EXPECT_TRUE(handlers.contains("config/group-7/interval"));
  EXPECT_TRUE(handlers.contains("config/group-7/state"));
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

TEST_F(MqttSubscriberTest, HandsAValidIntervalOn) {
  handlers["config/group-7/interval"](R"({"seconds": 10})");
  handlers["config/group-7/interval"](R"({"seconds": "ten"})"); // dropped
  ASSERT_EQ(intervals.size(), 1U);
  EXPECT_EQ(intervals[0], std::chrono::seconds{10});
}

TEST_F(MqttSubscriberTest, HandsAValidForcedStateOn) {
  handlers["config/group-7/state"](R"({"state": "alert"})");
  handlers["config/group-7/state"](R"({"state": "purple"})"); // dropped
  ASSERT_EQ(states.size(), 1U);
  EXPECT_EQ(states[0], State::Alert);
}

TEST_F(MqttSubscriberTest, TakesAnEmptyMessageAsAClearedCommand) {
  handlers["config/group-7/temperature-threshold"]("");
  EXPECT_TRUE(temperatureThresholds.empty());
}
