#pragma once

#include "State.h"
#include "StateMachine.h"

#include <chrono>

namespace warden {

/// Where warden reports what it measures, for others to see. The core does not know where that is.
class Reporter {
public:
  Reporter()                            = default;
  virtual ~Reporter()                   = default;
  Reporter(const Reporter &)            = delete;
  Reporter &operator=(const Reporter &) = delete;
  Reporter(Reporter &&)                 = delete;
  Reporter &operator=(Reporter &&)      = delete;

  /// Reports the averaged temperature and humidity, read at `time`.
  virtual void reportReading(const SmoothedReading &reading, std::chrono::system_clock::time_point time) = 0;

  /// Reports the state warden is in, entered at `time`.
  virtual void reportState(State state, std::chrono::system_clock::time_point time) = 0;
};

} // namespace warden
