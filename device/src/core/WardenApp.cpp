#include "WardenApp.h"

#include <spdlog/spdlog.h>

#include <condition_variable>
#include <mutex>
#include <stop_token>
#include <thread>

namespace warden {

namespace {

/// Combines lambdas into one callable with an overload per event type, for std::visit(): each event
/// type gets the lambda that takes it.
template <typename... Handlers> struct Overloaded : Handlers... {
  using Handlers::operator()...;
};

} // namespace

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
  while (auto event = m_events.pop(appStopToken)) {
    std::visit(
        Overloaded{
            [this](const SensorResult &result) { m_processor.process(result); },
            [this](ReportStateAgain /*request*/) { m_processor.reportState(); },
            [this](const NewTemperatureThreshold &command) { m_processor.setTemperatureThreshold(command.threshold); },
            [this](const NewHumidityThreshold &command) { m_processor.setHumidityThreshold(command.threshold); },
            [this](const NewInterval &command) { applyReadInterval(command.interval); },
            [this](const NewForcedState &command) { m_processor.forceState(command.state); },
        },
        *event);
  }

  m_led.turnOff();
  m_buzzer.turnOff();
}

void WardenApp::reportStateAgain() { m_events.push(ReportStateAgain{}); }

void WardenApp::setTemperatureThreshold(float threshold) {
  m_events.push(NewTemperatureThreshold{.threshold = threshold});
}

void WardenApp::setHumidityThreshold(float threshold) { m_events.push(NewHumidityThreshold{.threshold = threshold}); }

void WardenApp::setReadInterval(std::chrono::seconds interval) { m_events.push(NewInterval{.interval = interval}); }

void WardenApp::forceState(State state) { m_events.push(NewForcedState{.state = state}); }

void WardenApp::applyReadInterval(std::chrono::seconds interval) {
  if (!isUsableReadInterval(interval)) {
    spdlog::warn("Ignored the new read interval {} s: it must be between {} and {} s", interval.count(),
                 minReadInterval.count(), maxReadInterval.count());
    return;
  }
  {
    const std::scoped_lock lock{m_intervalMutex};
    m_readInterval = interval;
  }
  spdlog::info("New read interval: {} s", interval.count());
  m_intervalChanged.notify_one();
}

void WardenApp::sensorLoop(const std::stop_token &stopToken) {
  while (!stopToken.stop_requested()) {
    m_events.push(m_sensor.read());

    const auto lastRead = std::chrono::steady_clock::now();

    // Sleeps after failed reads too, so broken hardware is not hammered. C++20 has no sleep_for()
    // that takes a stop_token, but condition_variable_any does: a stop request ends the wait. So
    // does a new interval, which then counts from the last read, so the DHT22 still gets its 2 s
    // between reads.
    std::unique_lock lock{m_intervalMutex};
    auto interval = m_readInterval;
    while (m_intervalChanged.wait_until(lock, stopToken, lastRead + interval,
                                        [this, interval] { return m_readInterval != interval; })) {
      interval = m_readInterval;
    }
  }
}

} // namespace warden
