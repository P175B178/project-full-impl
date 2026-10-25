#pragma once

#include "State.h"
#include "StateMachine.h"
#include "interfaces/Buzzer.h"
#include "interfaces/Led.h"

namespace warden {

/// Signals a state transition: the LED shows the new state, and the buzzer beeps when entering or
/// leaving Alert.
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
  }

  if (transition.to == State::Alert) {
    buzzer.shortBeep(3U);
  } else if (transition.from == State::Alert) {
    buzzer.longBeep(1U);
  }
}

} // namespace warden
