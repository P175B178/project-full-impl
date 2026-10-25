#include "WardenApp.h"

#include <condition_variable>
#include <mutex>
#include <stop_token>
#include <thread>

namespace warden {

WardenApp::WardenApp(Sensor &sensor, Led &led, Buzzer &buzzer, Reporter &reporter, const Config &config)
      : m_sensor{sensor}
      , m_led{led}
      , m_buzzer{buzzer}
      , m_readInterval{config.readInterval}
      , m_processor{led, buzzer, reporter, config} {}

void WardenApp::run(const std::stop_token &appStopToken) {
  m_processor.start();

  // The jthread destructor requests stop and joins the thread when run() returns.
  auto sensorThreadFn = [this](const std::stop_token &threadStopToken) { sensorLoop(threadStopToken); };
  std::jthread sensorThread{sensorThreadFn};

  // pop() returns std::nullopt once the app requests stop, ending the loop.
  while (auto result = m_resultQueue.pop(appStopToken)) {
    m_processor.process(*result);
  }

  m_led.turnOff();
  m_buzzer.turnOff();
}

void WardenApp::sensorLoop(const std::stop_token &stopToken) {
  // C++20 has no sleep_for() that takes a stop_token, but condition_variable_any::wait_for() does:
  // a stop request ends the wait. The predicate is never true, so nothing else does.
  std::mutex sleepMutex;
  std::condition_variable_any sleepCv;

  while (!stopToken.stop_requested()) {
    m_resultQueue.push(m_sensor.read());

    // Sleeps after failed reads too, so broken hardware is not hammered.
    std::unique_lock lock{sleepMutex};
    sleepCv.wait_for(lock, stopToken, m_readInterval, [] { return false; });
  }
}

} // namespace warden
