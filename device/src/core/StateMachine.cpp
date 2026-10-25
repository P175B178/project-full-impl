#include "StateMachine.h"

#include <spdlog/spdlog.h>

namespace warden {

StateMachine::StateMachine(Thresholds thresholds)
      : m_thresholds{thresholds} {}

std::optional<StateTransition> StateMachine::update(const Reading &reading) {
  const State newState = evaluate(reading);
  if (newState == m_state) {
    return std::nullopt;
  }

  StateTransition transition{.from = m_state, .to = newState};
  m_state = newState;
  return transition;
}

State StateMachine::evaluate(const Reading &reading) const noexcept {
  const bool temperatureExceeded = reading.temperature > m_thresholds.temperature;
  const bool humidityExceeded    = reading.humidity > m_thresholds.humidity;

  spdlog::debug("StateMachine: temp={:.1f} (threshold={:.1f}, exceeded={}) hum={:.1f} (threshold={:.1f}, exceeded={})",
                reading.temperature, m_thresholds.temperature, temperatureExceeded, reading.humidity,
                m_thresholds.humidity, humidityExceeded);

  if (temperatureExceeded && humidityExceeded) {
    return State::Alert;
  }
  if (temperatureExceeded || humidityExceeded) {
    return State::Warning;
  }
  return State::Normal;
}

} // namespace warden
