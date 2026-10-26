#include "messages/Topics.h"

#include <gtest/gtest.h>

using namespace warden::messages;

TEST(TopicsTest, FollowTheProtocol) {
  EXPECT_EQ(topics::temperature("group-1"), "sensors/group-1/temperature");
  EXPECT_EQ(topics::humidity("group-1"), "sensors/group-1/humidity");
  EXPECT_EQ(topics::status("group-1"), "sensors/group-1/status");
  EXPECT_EQ(topics::temperatureThreshold("group-1"), "config/group-1/temperature-threshold");
  EXPECT_EQ(topics::humidityThreshold("group-1"), "config/group-1/humidity-threshold");
  EXPECT_EQ(topics::interval("group-1"), "config/group-1/interval");
  EXPECT_EQ(topics::forcedState("group-1"), "config/group-1/state");
}

TEST(TopicsTest, AnyGroupMakesAFilterForEveryGroup) { EXPECT_EQ(topics::status(topics::anyGroup), "sensors/+/status"); }
