#pragma once

#include "interfaces/Buzzer.h"

#include <spdlog/spdlog.h>

namespace warden::sim {

/// Buzzer for the simulation: prints what a real buzzer would play.
class ConsoleBuzzer final : public Buzzer {
public:
  void shortBeep(unsigned int count) override { spdlog::info("[BUZZER] {} short beep(s)", count); }

  void longBeep(unsigned int count) override { spdlog::info("[BUZZER] {} long beep(s)", count); }

  void turnOff() override { spdlog::info("[BUZZER] off"); }
};

} // namespace warden::sim
