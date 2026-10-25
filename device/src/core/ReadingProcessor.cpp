#include "ReadingProcessor.h"

#include "SignalTransition.h"

#include <spdlog/spdlog.h>

namespace warden {

ReadingProcessor::ReadingProcessor(Led &led, Buzzer &buzzer, const Config &config)
      : m_led{led}
      , m_buzzer{buzzer}
      , m_plausibleValues{config.plausibleValues}
      , m_stateMachine{config.thresholds}
      , m_temperatureBuffer{config.averagingWindow}
      , m_humidityBuffer{config.averagingWindow} {}

void ReadingProcessor::process(const std::expected<Reading, SensorError> &result) {
  if (!result) {
    spdlog::warn("Sensor read failed ({}) — skipping reading", toString(result.error()));
    return;
  }
  const Reading &reading = *result;

  // Skips what the sensor can report but is implausible here, as set in [plausible_values].
  if (!m_plausibleValues.temperature.contains(reading.temperature) ||
      !m_plausibleValues.humidity.contains(reading.humidity)) {
    spdlog::warn("Reading outside plausible range (temp={:.1f}, hum={:.1f}) — skipping", reading.temperature,
                 reading.humidity);
    return;
  }

  m_temperatureBuffer.push(reading.temperature);
  m_humidityBuffer.push(reading.humidity);

  // Never empty right after push(): value_or() only avoids an unchecked access.
  const SmoothedReading smoothed{.temperature = m_temperatureBuffer.average().value_or(reading.temperature),
                                 .humidity    = m_humidityBuffer.average().value_or(reading.humidity)};

  const auto transition = m_stateMachine.update(smoothed);

  spdlog::info("temp={:.1f} °C (raw={:.1f})  hum={:.1f}% (raw={:.1f})  state={}", smoothed.temperature,
               reading.temperature, smoothed.humidity, reading.humidity, toString(m_stateMachine.state()));
  if (transition) {
    spdlog::warn("State change: {} -> {}", toString(transition->from), toString(transition->to));
    signalTransition(m_led, m_buzzer, *transition);
  }
}

} // namespace warden
