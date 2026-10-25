#include "WardenApp.h"

#include <condition_variable>
#include <mutex>
#include <stop_token>

namespace warden {

WardenApp::WardenApp(Sensor &sensor, Led &led, Buzzer &buzzer, const Config &config)
      : m_sensor{sensor}
      , m_led{led}
      , m_buzzer{buzzer}
      , m_readInterval{config.readInterval}
      , m_processor{led, buzzer, config} {}

void WardenApp::run(const std::stop_token &stopToken) {
  m_led.show(LedColor::Green, LedMode::Steady);

  // C++20 has no sleep_for() that takes a stop_token, but condition_variable_any::wait_for() does:
  // a stop request ends the wait. The predicate is never true, so nothing else does.
  std::mutex sleepMutex;
  std::condition_variable_any sleepCv;

  while (!stopToken.stop_requested()) {
    m_processor.process(m_sensor.read());

    // Sleeps after failed reads too, so broken hardware is not hammered.
    std::unique_lock lock{sleepMutex};
    sleepCv.wait_for(lock, stopToken, m_readInterval, [] { return false; });
  }

  m_led.turnOff();
  m_buzzer.turnOff();
}

} // namespace warden
