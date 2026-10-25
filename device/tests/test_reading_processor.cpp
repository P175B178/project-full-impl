#include "ReadingProcessor.h"

#include "mocks/MockBuzzer.h"
#include "mocks/MockLed.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <expected>

using namespace warden;
using namespace warden::test;
using ::testing::InSequence;
using ::testing::StrictMock;

namespace {

/// The default config (thresholds 28 C / 70 %, plausible temperature 0-50 C) with the given
/// averaging window.
Config configWithWindow(std::size_t window) {
  Config config;
  config.averagingWindow = window;
  return config;
}

const Reading comfortable{.temperature = 20.0F, .humidity = 50.0F};
const Reading hotAndHumid{.temperature = 30.0F, .humidity = 80.0F};
const Reading implausible{.temperature = 70.0F, .humidity = 50.0F}; // above the plausible 50 C
const std::expected<Reading, SensorError> readFailure = std::unexpected{SensorError::ReadFailure};

/// The default config with a sensor fault after the given number of failed reads.
Config configWithFaultAfter(std::size_t failedReads) {
  Config config;
  config.faultAfterFailedReads = failedReads;
  return config;
}

} // namespace

// StrictMock fails the test on any LED or buzzer call that is not expected, so a test without
// EXPECT_CALLs checks that nothing reacted at all.
class ReadingProcessorTest : public ::testing::Test {
protected:
  StrictMock<MockLed> m_led;
  StrictMock<MockBuzzer> m_buzzer;
};

TEST_F(ReadingProcessorTest, StaysQuietWhileReadingsAreBelowThresholds) {
  ReadingProcessor processor{m_led, m_buzzer, configWithWindow(1)};
  processor.process(comfortable);
  processor.process(comfortable);
  EXPECT_EQ(processor.state(), State::Normal);
}

TEST_F(ReadingProcessorTest, EnteringAlertBlinksRedAndBeeps) {
  ReadingProcessor processor{m_led, m_buzzer, configWithWindow(1)};
  EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Blinking));
  EXPECT_CALL(m_buzzer, shortBeep(3));
  processor.process(hotAndHumid);
  EXPECT_EQ(processor.state(), State::Alert);
}

TEST_F(ReadingProcessorTest, LeavingAlertTurnsGreenAndGivesOneLongBeep) {
  ReadingProcessor processor{m_led, m_buzzer, configWithWindow(1)};
  {
    const InSequence inOrder;
    EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Blinking));
    EXPECT_CALL(m_buzzer, shortBeep(3));
    EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
    EXPECT_CALL(m_buzzer, longBeep(1));
  }
  processor.process(hotAndHumid);
  processor.process(comfortable);
  EXPECT_EQ(processor.state(), State::Normal);
}

TEST_F(ReadingProcessorTest, ReactsToTheAverageNotToASingleReading) {
  ReadingProcessor processor{m_led, m_buzzer, configWithWindow(2)};
  processor.process(Reading{.temperature = 20.0F, .humidity = 50.0F});
  processor.process(Reading{.temperature = 34.0F, .humidity = 50.0F}); // average 27 C: still Normal
  EXPECT_EQ(processor.state(), State::Normal);

  EXPECT_CALL(m_led, show(LedColor::Orange, LedMode::Steady));
  processor.process(Reading{.temperature = 34.0F, .humidity = 50.0F}); // average 34 C: Warning
  EXPECT_EQ(processor.state(), State::Warning);
}

TEST_F(ReadingProcessorTest, SkipsImplausibleReadingsWithoutAveragingThem) {
  ReadingProcessor processor{m_led, m_buzzer, configWithWindow(2)};
  processor.process(Reading{.temperature = 20.0F, .humidity = 50.0F});
  processor.process(Reading{.temperature = 70.0F, .humidity = 50.0F}); // above the plausible 50 C: skipped
  processor.process(Reading{.temperature = 30.0F, .humidity = 50.0F}); // average of 20 and 30 is 25 C
  // Had the 70 C reading been averaged in, the average would be 50 C: Warning, and StrictMock would
  // fail the test on the unexpected LED call.
  EXPECT_EQ(processor.state(), State::Normal);
}

TEST_F(ReadingProcessorTest, SkipsFailedReads) {
  ReadingProcessor processor{m_led, m_buzzer, configWithWindow(1)};
  processor.process(readFailure); // StrictMock fails the test on any LED or buzzer call
  EXPECT_EQ(processor.state(), State::Normal);
}

TEST_F(ReadingProcessorTest, StaysQuietWhileFailedReadsStayBelowTheLimit) {
  ReadingProcessor processor{m_led, m_buzzer, configWithFaultAfter(3)};
  processor.process(readFailure);
  processor.process(implausible);
  EXPECT_EQ(processor.state(), State::Normal);
}

TEST_F(ReadingProcessorTest, RepeatedFailedReadsShowASteadyRedFault) {
  ReadingProcessor processor{m_led, m_buzzer, configWithFaultAfter(3)};
  EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Steady));
  processor.process(readFailure);
  processor.process(implausible); // implausible readings count as failed reads too
  processor.process(readFailure);
  EXPECT_EQ(processor.state(), State::Fault);
}

TEST_F(ReadingProcessorTest, AValidReadingResetsTheFailedReadCount) {
  ReadingProcessor processor{m_led, m_buzzer, configWithFaultAfter(3)};
  processor.process(readFailure);
  processor.process(readFailure);
  processor.process(comfortable);
  processor.process(readFailure);
  processor.process(readFailure);
  EXPECT_EQ(processor.state(), State::Normal);
}

TEST_F(ReadingProcessorTest, AValidReadingEndsTheFault) {
  ReadingProcessor processor{m_led, m_buzzer, configWithFaultAfter(1)};
  {
    const InSequence inOrder;
    EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Steady));
    EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
  }
  processor.process(readFailure);
  processor.process(comfortable);
  EXPECT_EQ(processor.state(), State::Normal);
}

TEST_F(ReadingProcessorTest, FaultDuringAlertGivesNoAllClearBeep) {
  ReadingProcessor processor{m_led, m_buzzer, configWithFaultAfter(1)};
  {
    const InSequence inOrder;
    EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Blinking));
    EXPECT_CALL(m_buzzer, shortBeep(3));
    EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Steady));
  }
  processor.process(hotAndHumid);
  processor.process(readFailure); // StrictMock fails the test on a longBeep()
}

TEST_F(ReadingProcessorTest, AFaultEndsAnAlarmWithoutTheLongBeep) {
  ReadingProcessor processor{m_led, m_buzzer, configWithFaultAfter(1)};
  {
    const InSequence inOrder;
    EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Blinking));
    EXPECT_CALL(m_buzzer, shortBeep(3));
    EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Steady));
    EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
  }
  processor.process(hotAndHumid);
  processor.process(readFailure);
  processor.process(comfortable); // as after startup: StrictMock fails the test on a longBeep()
  EXPECT_EQ(processor.state(), State::Normal);
}

TEST_F(ReadingProcessorTest, AveragingStartsAfreshAfterAFault) {
  Config config          = configWithFaultAfter(1);
  config.averagingWindow = 2;
  ReadingProcessor processor{m_led, m_buzzer, config};
  processor.process(comfortable);
  {
    const InSequence inOrder;
    EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Steady));
    EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Blinking));
    EXPECT_CALL(m_buzzer, shortBeep(3));
  }
  processor.process(readFailure);
  // Averaged with the comfortable reading from before the fault, this would be 25 C and 65 %:
  // Normal. On its own, it is Alert.
  processor.process(hotAndHumid);
  EXPECT_EQ(processor.state(), State::Alert);
}
