#pragma once

#include "BlockingQueue.h"
#include "Config.h"
#include "ReadingProcessor.h"
#include "Reporter.h"
#include "State.h"
#include "interfaces/Buzzer.h"
#include "interfaces/Led.h"
#include "interfaces/Reading.h"
#include "interfaces/Sensor.h"

#include <chrono>
#include <condition_variable>
#include <expected>
#include <mutex>
#include <stop_token>
#include <variant>

namespace warden {

/// Runs the monitoring loop: reads the sensor and hands the result to a ReadingProcessor. The logic
/// lives there, where it can be tested on its own.
///
/// The sensor, LED, buzzer and reporter are injected, so any conforming implementation can be used.
///
/// Threading:
///   - sensorLoop() runs on its own thread and pushes each result onto the queue.
///   - Other threads push onto the same queue, through reportStateAgain() and the commands.
///   - run() takes the events off on the caller's thread and hands them to the
///     processor. Everything the processor does happens on that thread: no locks.
///   - run() stops and joins the sensor thread before returning.
class WardenApp {
public:
  WardenApp(Sensor &sensor, Led &led, Buzzer &buzzer, Reporter &reporter, const Config &config);

  ~WardenApp()                            = default;
  WardenApp(const WardenApp &)            = delete;
  WardenApp &operator=(const WardenApp &) = delete;
  WardenApp(WardenApp &&)                 = delete;
  WardenApp &operator=(WardenApp &&)      = delete;

  /// Run the monitoring loop until the stop token is signalled. Shows the starting state on entry,
  /// turns LED and buzzer off on exit.
  void run(const std::stop_token &stopToken);

  /// Reports the current state again, as soon as run() gets to it. From any thread.
  void reportStateAgain();

  /// Commands, from any thread: each is queued, and run() applies it.
  void setTemperatureThreshold(float threshold);
  void setHumidityThreshold(float threshold);
  void setReadInterval(std::chrono::seconds interval);
  void forceState(State state);

private:
  using SensorResult = std::expected<Reading, SensorError>;
  /// reportStateAgain() was called.
  struct ReportStateAgain {};
  /// A command set a threshold.
  struct NewTemperatureThreshold {
    float threshold;
  };
  struct NewHumidityThreshold {
    float threshold;
  };
  /// A command set the read interval.
  struct NewInterval {
    std::chrono::seconds interval;
  };
  /// A command forced a state until the next valid reading.
  struct NewForcedState {
    State state;
  };
  /// What run() reacts to: a read of the sensor, or a call from another thread.
  using Event = std::variant<SensorResult,
                             ReportStateAgain,
                             NewTemperatureThreshold,
                             NewHumidityThreshold,
                             NewInterval,
                             NewForcedState>;

  /// Applies a new read interval, if it is within the config file's limits; logs and ignores it
  /// otherwise.
  void applyReadInterval(std::chrono::seconds interval);
  /// Reads the sensor periodically and pushes each result onto events.
  void sensorLoop(const std::stop_token &stopToken);

  Sensor &m_sensor;
  Led &m_led;
  Buzzer &m_buzzer;
  /// Set by run()'s thread and read by the sensor thread, so guarded by m_intervalMutex. A change
  /// wakes the sensor thread through m_intervalChanged.
  std::chrono::seconds m_readInterval;
  std::mutex m_intervalMutex;
  std::condition_variable_any m_intervalChanged;
  ReadingProcessor m_processor;
  BlockingQueue<Event> m_events;
};

} // namespace warden
