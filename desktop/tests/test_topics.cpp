#include "messages/Topics.h"

#include <gtest/gtest.h>

using namespace warden::messages;

TEST(TopicsTest, FollowTheProtocol) {
  EXPECT_EQ(topics::temperature("group-1"), "sensors/group-1/temperature");
  EXPECT_EQ(topics::humidity("group-1"), "sensors/group-1/humidity");
  EXPECT_EQ(topics::status("group-1"), "sensors/group-1/status");
}
