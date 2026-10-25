#pragma once

#include "State.h"
#include "StateMachine.h"
#include "interfaces/Buzzer.h"
#include "interfaces/Led.h"

namespace warden {

/// Signals a state transition: the LED shows the new state, and the buzzer beeps when entering or
/// leaving Alert.
///
/// A fault is like switching off and on: entering it gives no beep, and leaving
/// it is signalled like the first reading after startup, so
/// Alert -> Fault -> Normal gives no long beep.
inline void signalTransition(Led &led, Buzzer &buzzer, const StateTransition &transition) {
  switch (transition.to) {
  case State::Normal:
    led.show(LedColor::Green, LedMode::Steady);
    break;
  case State::Warning:
    led.show(LedColor::Orange, LedMode::Steady);
    break;
  case State::Alert:
    led.show(LedColor::Red, LedMode::Blinking);
    break;
  case State::Fault:
    led.show(LedColor::Red, LedMode::Steady);
    break;
  }

  if (transition.to == State::Alert) {
    buzzer.shortBeep(3U);
  } else if (transition.from == State::Alert && transition.to != State::Fault) {
    buzzer.longBeep(1U);
  }
}

} // namespace warden
