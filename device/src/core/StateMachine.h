#pragma once

#include "State.h"
#include "Thresholds.h"

#include <optional>

namespace warden {

struct StateTransition {
  State from;
  State to;
};

/// Averaged readings: what the thresholds are checked against. A separate type, so a raw Reading
/// cannot be passed to the state machine by mistake.
struct SmoothedReading {
  float temperature{};
  float humidity{};
};

class StateMachine {
public:
  explicit StateMachine(Thresholds thresholds);

  [[nodiscard]] std::optional<StateTransition> update(SmoothedReading smoothed);
  /// Enters Fault. The next update() leaves it again.
  [[nodiscard]] std::optional<StateTransition> fault();
  /// Enters `state`, from a command. The next update() leaves it again.
  [[nodiscard]] std::optional<StateTransition> force(State state);
  [[nodiscard]] State state() const noexcept { return m_state; }

  [[nodiscard]] Thresholds thresholds() const noexcept { return m_thresholds; }
  /// Takes effect with the next update().
  void setThresholds(Thresholds newThresholds) noexcept { m_thresholds = newThresholds; }

private:
  [[nodiscard]] State evaluate(SmoothedReading smoothed) const noexcept;
  [[nodiscard]] std::optional<StateTransition> moveTo(State newState);

  Thresholds m_thresholds;
  State m_state{State::Normal};
};

} // namespace warden
