#include "StatusLed.h"

#include <spdlog/spdlog.h>

namespace warden::hardware {

void StatusLed::show(LedColor color, LedMode mode) {
  spdlog::debug("StatusLed (not implemented yet): show color={} blink={}", toString(color), mode == LedMode::Blinking);
}

void StatusLed::turnOff() { spdlog::debug("StatusLed (not implemented yet): turnOff"); }

} // namespace warden::hardware
