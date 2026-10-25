#pragma once

#include "AveragingBuffer.h"
#include "Config.h"
#include "Range.h"
#include "State.h"
#include "StateMachine.h"
#include "interfaces/Buzzer.h"
#include "interfaces/Led.h"
#include "interfaces/Reading.h"

namespace warden {

/// Reacts to the sensor: the device's logic, without threads, sleeps or sensor access, so it can be
/// unit tested with mocks.
class ReadingProcessor {
public:
  ReadingProcessor(Led &led, Buzzer &buzzer, const Config &config);

  ~ReadingProcessor()                                   = default;
  ReadingProcessor(const ReadingProcessor &)            = delete;
  ReadingProcessor &operator=(const ReadingProcessor &) = delete;
  ReadingProcessor(ReadingProcessor &&)                 = delete;
  ReadingProcessor &operator=(ReadingProcessor &&)      = delete;

  /// React to one raw reading from the sensor.
  void process(const Reading &reading);

  [[nodiscard]] State state() const noexcept { return m_stateMachine.state(); }

private:
  Led &m_led;
  Buzzer &m_buzzer;
  PlausibleValues m_plausibleValues;
  StateMachine m_stateMachine;
  AveragingBuffer<float> m_temperatureBuffer;
  AveragingBuffer<float> m_humidityBuffer;
};

} // namespace warden
