#include "SignalTransition.h"

#include "mocks/MockBuzzer.h"
#include "mocks/MockLed.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace warden;
using namespace warden::test;
using ::testing::StrictMock;

class SignalTransitionTest : public ::testing::Test {
protected:
  StrictMock<MockLed> m_led;
  StrictMock<MockBuzzer> m_buzzer;
};

TEST_F(SignalTransitionTest, NormalGreenSteadyLedNoBuzzer) {
  EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
  signalTransition(m_led, m_buzzer, {.from = State::Warning, .to = State::Normal});
}

TEST_F(SignalTransitionTest, WarningOrangeSteadyLedNoBuzzer) {
  EXPECT_CALL(m_led, show(LedColor::Orange, LedMode::Steady));
  signalTransition(m_led, m_buzzer, {.from = State::Normal, .to = State::Warning});
}

TEST_F(SignalTransitionTest, EnteringAlertRedBlinkingLedThreeShortBeeps) {
  EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Blinking));
  EXPECT_CALL(m_buzzer, shortBeep(3));
  signalTransition(m_led, m_buzzer, {.from = State::Normal, .to = State::Alert});
}

TEST_F(SignalTransitionTest, LeavingAlertOneLongBeep) {
  EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
  EXPECT_CALL(m_buzzer, longBeep(1));
  signalTransition(m_led, m_buzzer, {.from = State::Alert, .to = State::Normal});
}

TEST_F(SignalTransitionTest, WarningToAlertRedBlinkingLedThreeShortBeeps) {
  EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Blinking));
  EXPECT_CALL(m_buzzer, shortBeep(3));
  signalTransition(m_led, m_buzzer, {.from = State::Warning, .to = State::Alert});
}

TEST_F(SignalTransitionTest, AlertToWarningOrangeSteadyLedOneLongBeep) {
  EXPECT_CALL(m_led, show(LedColor::Orange, LedMode::Steady));
  EXPECT_CALL(m_buzzer, longBeep(1));
  signalTransition(m_led, m_buzzer, {.from = State::Alert, .to = State::Warning});
}
