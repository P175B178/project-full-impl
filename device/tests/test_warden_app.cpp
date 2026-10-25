#include "WardenApp.h"

#include "Config.h"
#include "interfaces/Reading.h"
#include "interfaces/Sensor.h"
#include "mocks/MockBuzzer.h"
#include "mocks/MockLed.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <expected>
#include <stop_token>

using namespace warden;
using namespace warden::test;
using ::testing::InSequence;
using ::testing::InvokeWithoutArgs;
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

} // namespace

class WardenAppTest : public ::testing::Test {
protected:
  StrictMock<MockLed> m_led;
  StrictMock<MockBuzzer> m_buzzer;
  std::stop_source m_stop;
  Config m_config;
};

TEST_F(WardenAppTest, TurnsEverythingOffWhenStopped) {
  FakeSensor sensor{Reading{.temperature = 20.0F, .humidity = 50.0F}};
  WardenApp app{sensor, m_led, m_buzzer, m_config};
  const InSequence inOrder;
  EXPECT_CALL(m_led, show(LedColor::Green, LedMode::Steady));
  EXPECT_CALL(m_led, turnOff());
  EXPECT_CALL(m_buzzer, turnOff());

  m_stop.request_stop();
  app.run(m_stop.get_token());
}

TEST_F(WardenAppTest, HandsReadingsToTheProcessor) {
  FakeSensor sensor{Reading{.temperature = 30.0F, .humidity = 80.0F}};
  WardenApp app{sensor, m_led, m_buzzer, m_config};
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
