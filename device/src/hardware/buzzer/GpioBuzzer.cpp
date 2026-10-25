#include "GpioBuzzer.h"

#include <spdlog/spdlog.h>

namespace warden::hardware {

void GpioBuzzer::shortBeep(unsigned int count) {
  spdlog::debug("GpioBuzzer (not implemented yet): {} short beep(s)", count);
}

void GpioBuzzer::longBeep(unsigned int count) {
  spdlog::debug("GpioBuzzer (not implemented yet): {} long beep(s)", count);
}

void GpioBuzzer::turnOff() { spdlog::debug("GpioBuzzer (not implemented yet): turnOff"); }

} // namespace warden::hardware
