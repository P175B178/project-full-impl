#include "StateMachine.h"

#include <gtest/gtest.h>

using namespace warden;

static constexpr float temperatureThreshold = 28.0F;
static constexpr float humidityThreshold    = 70.0F;
static constexpr Thresholds thresholds{.temperature = temperatureThreshold, .humidity = humidityThreshold};

TEST(StateMachineTest, StartsInNormal) {
  StateMachine sm{thresholds};
  EXPECT_EQ(sm.state(), State::Normal);
}

TEST(StateMachineTest, StaysNormalWhenBothThresholdsNotExceeded) {
  StateMachine sm{thresholds};
  auto t = sm.update({.temperature = 20.0F, .humidity = 50.0F});
  EXPECT_FALSE(t.has_value());
  EXPECT_EQ(sm.state(), State::Normal);
}

TEST(StateMachineTest, NormalToWarningWhenTemperatureOnlyExceeded) {
  StateMachine sm{thresholds};
  auto t = sm.update({.temperature = 30.0F, .humidity = 50.0F});
  ASSERT_TRUE(t.has_value());
  EXPECT_EQ(t->from, State::Normal);
  EXPECT_EQ(t->to, State::Warning);
  EXPECT_EQ(sm.state(), State::Warning);
}

TEST(StateMachineTest, NormalToWarningWhenHumidityOnlyExceeded) {
  StateMachine sm{thresholds};
  auto t = sm.update({.temperature = 20.0F, .humidity = 80.0F});
  ASSERT_TRUE(t.has_value());
  EXPECT_EQ(t->to, State::Warning);
}

TEST(StateMachineTest, NormalToAlertWhenBothThresholdsExceeded) {
  StateMachine sm{thresholds};
  auto t = sm.update({.temperature = 30.0F, .humidity = 80.0F});
  ASSERT_TRUE(t.has_value());
  EXPECT_EQ(t->from, State::Normal);
  EXPECT_EQ(t->to, State::Alert);
  EXPECT_EQ(sm.state(), State::Alert);
}

TEST(StateMachineTest, WarningToAlertWhenSecondThresholdExceeded) {
  StateMachine sm{thresholds};
  std::ignore = sm.update({.temperature = 30.0F, .humidity = 50.0F}); // -> Warning
  auto t      = sm.update({.temperature = 30.0F, .humidity = 80.0F});
  ASSERT_TRUE(t.has_value());
  EXPECT_EQ(t->from, State::Warning);
  EXPECT_EQ(t->to, State::Alert);
}

TEST(StateMachineTest, AlertToWarningWhenOneThresholdDropsBelow) {
  StateMachine sm{thresholds};
  std::ignore = sm.update({.temperature = 30.0F, .humidity = 80.0F}); // -> Alert
  auto t      = sm.update({.temperature = 20.0F, .humidity = 80.0F});
  ASSERT_TRUE(t.has_value());
  EXPECT_EQ(t->from, State::Alert);
  EXPECT_EQ(t->to, State::Warning);
}

TEST(StateMachineTest, AlertToNormalWhenBothThresholdsDropBelow) {
  StateMachine sm{thresholds};
  std::ignore = sm.update({.temperature = 30.0F, .humidity = 80.0F}); // -> Alert
  auto t      = sm.update({.temperature = 20.0F, .humidity = 50.0F});
  ASSERT_TRUE(t.has_value());
  EXPECT_EQ(t->from, State::Alert);
  EXPECT_EQ(t->to, State::Normal);
}

TEST(StateMachineTest, NoTransitionWhenStateStaysTheSame) {
  StateMachine sm{thresholds};
  std::ignore = sm.update({.temperature = 30.0F, .humidity = 80.0F}); // -> Alert
  auto t      = sm.update({.temperature = 31.0F, .humidity = 85.0F}); // still Alert
  EXPECT_FALSE(t.has_value());
}

TEST(StateMachineTest, ValuesExactlyAtThresholdAreNotExceeded) {
  StateMachine sm{thresholds};
  auto t = sm.update({.temperature = temperatureThreshold, .humidity = humidityThreshold});
  EXPECT_FALSE(t.has_value());
  EXPECT_EQ(sm.state(), State::Normal);
}

TEST(StateMachineTest, FaultIsEnteredFromAnyState) {
  StateMachine sm{thresholds};
  std::ignore = sm.update({.temperature = 30.0F, .humidity = 50.0F}); // -> Warning
  auto t      = sm.fault();
  ASSERT_TRUE(t.has_value());
  EXPECT_EQ(t->from, State::Warning);
  EXPECT_EQ(t->to, State::Fault);
  EXPECT_EQ(sm.state(), State::Fault);
}

TEST(StateMachineTest, FaultIsEnteredOnlyOnce) {
  StateMachine sm{thresholds};
  std::ignore = sm.fault();
  EXPECT_FALSE(sm.fault().has_value());
}

TEST(StateMachineTest, UpdateLeavesFault) {
  StateMachine sm{thresholds};
  std::ignore = sm.fault();
  auto t      = sm.update({.temperature = 20.0F, .humidity = 50.0F});
  ASSERT_TRUE(t.has_value());
  EXPECT_EQ(t->from, State::Fault);
  EXPECT_EQ(t->to, State::Normal);
}
