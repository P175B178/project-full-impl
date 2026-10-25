#include "StateMachine.h"

#include <spdlog/spdlog.h>

namespace warden {

StateMachine::StateMachine(Thresholds thresholds)
      : m_thresholds{thresholds} {}

std::optional<StateTransition> StateMachine::update(SmoothedReading smoothed) { return moveTo(evaluate(smoothed)); }

std::optional<StateTransition> StateMachine::fault() { return moveTo(State::Fault); }

std::optional<StateTransition> StateMachine::force(State state) { return moveTo(state); }

std::optional<StateTransition> StateMachine::moveTo(State newState) {
  if (newState == m_state) {
    return std::nullopt;
  }

  StateTransition transition{.from = m_state, .to = newState};
  m_state = newState;
  return transition;
}

State StateMachine::evaluate(SmoothedReading smoothed) const noexcept {
  const bool temperatureExceeded = smoothed.temperature > m_thresholds.temperature;
  const bool humidityExceeded    = smoothed.humidity > m_thresholds.humidity;

  spdlog::debug("StateMachine: temp={:.1f} (threshold={:.1f}, exceeded={}) hum={:.1f} (threshold={:.1f}, exceeded={})",
                smoothed.temperature, m_thresholds.temperature, temperatureExceeded, smoothed.humidity,
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
