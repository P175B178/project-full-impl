#pragma once

#include "State.h"
#include "Thresholds.h"
#include "interfaces/Reading.h"

#include <optional>

namespace warden {

struct StateTransition {
  State from;
  State to;
};

class StateMachine {
public:
  explicit StateMachine(Thresholds thresholds);

  [[nodiscard]] std::optional<StateTransition> update(const Reading &reading);
  [[nodiscard]] State state() const noexcept { return m_state; }

private:
  [[nodiscard]] State evaluate(const Reading &reading) const noexcept;

  Thresholds m_thresholds;
  State m_state{State::Normal};
};

} // namespace warden
