#include "WardenApp.h"

#include "Config.h"
#include "interfaces/Reading.h"
#include "interfaces/Sensor.h"
#include "mocks/MockBuzzer.h"
#include "mocks/MockLed.h"
#include "mocks/MockReporter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <expected>
#include <stop_token>
#include <utility>

using namespace warden;
using namespace warden::test;
using ::testing::InSequence;
using ::testing::InvokeWithoutArgs;
using ::testing::NiceMock;
using ::testing::StrictMock;

namespace {

/// Gives the same result at every read.
class FakeSensor final : public Sensor {
public:
  explicit FakeSensor(const std::expected<Reading, SensorError> &result)
        : m_result{result} {}

  [[nodiscard]] std::expected<Reading, SensorError> read() override { return m_result; }

private:
  std::expected<Reading, SensorError> m_result;
};

/// Requests a stop at its second read.
class SecondReadStops final : public Sensor {
public:
  explicit SecondReadStops(std::stop_source stop)
        : m_stop{std::move(stop)} {}

  [[nodiscard]] std::expected<Reading, SensorError> read() override {
    if (++m_reads == 2) {
      m_stop.request_stop();
    }
    return Reading{.temperature = 20.0F, .humidity = 50.0F};
  }

private:
  std::stop_source m_stop;
  std::atomic<int> m_reads{0};
};

} // namespace

class WardenAppTest : public ::testing::Test {
protected:
  StrictMock<MockLed> m_led;
  StrictMock<MockBuzzer> m_buzzer;
  NiceMock<MockReporter> m_reporter;
  std::stop_source m_stop;
  Config m_config;
};

TEST_F(WardenAppTest, TurnsEverythingOffWhenStopped) {
  FakeSensor sensor{Reading{.temperature = 20.0F, .humidity = 50.0F}};
  WardenApp app{sensor, m_led, m_buzzer, m_reporter, m_config};
  const InSequence inOrder;
  EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
  EXPECT_CALL(m_led, turnOff());
  EXPECT_CALL(m_buzzer, turnOff());

  m_stop.request_stop();
  app.run(m_stop.get_token());
}

TEST_F(WardenAppTest, HandsReadingsToTheProcessor) {
  FakeSensor sensor{Reading{.temperature = 30.0F, .humidity = 80.0F}};
  WardenApp app{sensor, m_led, m_buzzer, m_reporter, m_config};
  const InSequence inOrder;
  EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
  EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Blinking)).WillOnce(InvokeWithoutArgs([this] {
    m_stop.request_stop();
  }));
  EXPECT_CALL(m_buzzer, shortBeep(3));
  EXPECT_CALL(m_led, turnOff());
  EXPECT_CALL(m_buzzer, turnOff());

  app.run(m_stop.get_token());
}

TEST_F(WardenAppTest, HandsFailedReadsToTheProcessor) {
  m_config.faultAfterFailedReads = 1;
  FakeSensor sensor{std::unexpected{SensorError::ReadFailure}};
  WardenApp app{sensor, m_led, m_buzzer, m_reporter, m_config};
  const InSequence inOrder;
  EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
  EXPECT_CALL(m_led, show(LedColor::Red, LedMode::Steady)).WillOnce(InvokeWithoutArgs([this] {
    m_stop.request_stop();
  }));
  EXPECT_CALL(m_led, turnOff());
  EXPECT_CALL(m_buzzer, turnOff());

  app.run(m_stop.get_token());
}

TEST_F(WardenAppTest, ReportsTheStateAgainWhenAsked) {
  FakeSensor sensor{Reading{.temperature = 20.0F, .humidity = 50.0F}};
  WardenApp app{sensor, m_led, m_buzzer, m_reporter, m_config};
  EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
  EXPECT_CALL(m_led, turnOff());
  EXPECT_CALL(m_buzzer, turnOff());
  EXPECT_CALL(m_reporter, reportState(State::Normal, ::testing::_)).WillOnce(InvokeWithoutArgs([this] {
    m_stop.request_stop();
  }));

  app.reportStateAgain();
  app.run(m_stop.get_token());
}

TEST_F(WardenAppTest, ANewIntervalCountsFromTheLastRead) {
  m_config.readInterval = maxReadInterval;
  SecondReadStops sensor{m_stop};
  WardenApp app{sensor, m_led, m_buzzer, m_reporter, m_config};
  EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
  EXPECT_CALL(m_led, turnOff());
  EXPECT_CALL(m_buzzer, turnOff());

  app.setReadInterval(minReadInterval);
  const auto start = std::chrono::steady_clock::now();
  app.run(m_stop.get_token());
  EXPECT_GE(std::chrono::steady_clock::now() - start, minReadInterval);
}
