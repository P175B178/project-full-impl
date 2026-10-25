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
  [[nodiscard]] State state() const noexcept { return m_state; }

private:
  [[nodiscard]] State evaluate(SmoothedReading smoothed) const noexcept;

  Thresholds m_thresholds;
  State m_state{State::Normal};
};

} // namespace warden
