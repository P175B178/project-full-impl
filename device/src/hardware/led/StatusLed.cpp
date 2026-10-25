#include "StatusLed.h"

#include <spdlog/spdlog.h>

namespace warden::hardware {

StatusLed::StatusLed() = default;

// turnOff() is virtual, but StatusLed is final, so this calls the version below.
StatusLed::~StatusLed() { turnOff(); }

void StatusLed::clearColor() {
  m_greenLed.turnOff();
  m_redLed.turnOff();
}

void StatusLed::applyColor(LedColor color) {
  switch (color) {
  case LedColor::Green:
    m_greenLed.turnOn();
    m_redLed.turnOff();
    break;
  case LedColor::Orange:
    m_greenLed.turnOn();
    m_redLed.turnOn();
    break;
  case LedColor::Red:
    m_greenLed.turnOff();
    m_redLed.turnOn();
    break;
  }
}

void StatusLed::show(LedColor color, LedMode mode) {
  spdlog::debug("StatusLed: show color={} blink={}", toString(color), mode == LedMode::Blinking);
  applyColor(color);
}

void StatusLed::turnOff() { clearColor(); }

} // namespace warden::hardware
