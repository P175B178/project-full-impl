#pragma once

#include "BlockingQueue.h"
#include "Config.h"
#include "ReadingProcessor.h"
#include "Reporter.h"
#include "interfaces/Buzzer.h"
#include "interfaces/Led.h"
#include "interfaces/Reading.h"
#include "interfaces/Sensor.h"

#include <chrono>
#include <expected>
#include <stop_token>

namespace warden {

/// Runs the monitoring loop: reads the sensor and hands the result to a ReadingProcessor. The logic
/// lives there, where it can be tested on its own.
///
/// The sensor, LED, buzzer and reporter are injected, so any conforming implementation can be used.
///
/// Threading:
///   - sensorLoop() runs on its own thread and pushes each result onto the queue.
///   - run() takes them off on the caller's thread and hands them to the processor.
///   - run() stops and joins the sensor thread before returning.
class WardenApp {
public:
  WardenApp(Sensor &sensor, Led &led, Buzzer &buzzer, Reporter &reporter, const Config &config);

  ~WardenApp()                            = default;
  WardenApp(const WardenApp &)            = delete;
  WardenApp &operator=(const WardenApp &) = delete;
  WardenApp(WardenApp &&)                 = delete;
  WardenApp &operator=(WardenApp &&)      = delete;

  /// Run the monitoring loop until the stop token is signalled. Sets the LED to green on entry,
  /// turns LED and buzzer off on exit.
  void run(const std::stop_token &stopToken);

private:
  /// Reads the sensor periodically and pushes each result onto resultQueue.
  void sensorLoop(const std::stop_token &stopToken);

  Sensor &m_sensor;
  Led &m_led;
  Buzzer &m_buzzer;
  std::chrono::seconds m_readInterval;
  ReadingProcessor m_processor;
  BlockingQueue<std::expected<Reading, SensorError>> m_resultQueue;
};

} // namespace warden
