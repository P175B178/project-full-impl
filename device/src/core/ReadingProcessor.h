#pragma once

#include "AveragingBuffer.h"
#include "Config.h"
#include "Range.h"
#include "Reporter.h"
#include "State.h"
#include "StateMachine.h"
#include "interfaces/Buzzer.h"
#include "interfaces/Led.h"
#include "interfaces/Reading.h"
#include "interfaces/Sensor.h"

#include <cstddef>
#include <expected>

namespace warden {

/// Reacts to the sensor: the device's logic, without threads, sleeps or sensor access, so it can be
/// unit tested with mocks.
class ReadingProcessor {
public:
  ReadingProcessor(Led &led, Buzzer &buzzer, Reporter &reporter, const Config &config);

  ~ReadingProcessor()                                   = default;
  ReadingProcessor(const ReadingProcessor &)            = delete;
  ReadingProcessor &operator=(const ReadingProcessor &) = delete;
  ReadingProcessor(ReadingProcessor &&)                 = delete;
  ReadingProcessor &operator=(ReadingProcessor &&)      = delete;

  /// Shows the state it starts in, Normal, on the LED. reportState() reports it.
  void start();

  /// Reports the current state, for whoever starts listening only now.
  void reportState();

  /// React to one read of the sensor: a reading, or the error it reported.
  void process(const std::expected<Reading, SensorError> &result);

  [[nodiscard]] State state() const noexcept { return m_stateMachine.state(); }

private:
  /// Counts a failed or implausible read; enough in a row mean Fault.
  void countFailedRead();

  /// Logs a state change, shows it on the LED and buzzer, and reports it.
  void reportTransition(const StateTransition &transition);

  Led &m_led;
  Buzzer &m_buzzer;
  Reporter &m_reporter;
  PlausibleValues m_plausibleValues;
  std::size_t m_faultAfterFailedReads;
  StateMachine m_stateMachine;
  AveragingBuffer<float> m_temperatureBuffer;
  AveragingBuffer<float> m_humidityBuffer;
  std::size_t m_failedReads{0}; ///< Failed or implausible reads in a row
};

} // namespace warden
