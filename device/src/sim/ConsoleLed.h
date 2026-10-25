#pragma once

#include "interfaces/Led.h"

#include <spdlog/spdlog.h>

namespace warden::sim {

/// LED for the simulation: prints what a real LED would show.
class ConsoleLed final : public Led {
public:
  void show(LedColor color, LedMode mode) override {
    spdlog::info("[LED] {}{}", toString(color), mode == LedMode::Blinking ? " (blinking)" : "");
  }

  void turnOff() override { spdlog::info("[LED] off"); }
};

} // namespace warden::sim
