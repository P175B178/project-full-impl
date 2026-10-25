#pragma once

#include "SysfsLed.h"
#include "interfaces/Led.h"

namespace warden::hardware {

/// Status LED, mixed from the Pi's built-in ACT (green) and PWR (red) LEDs:
///   Green  -> green LED on,  red LED off
///   Orange -> green LED on,  red LED on
///   Red    -> green LED off, red LED on
class StatusLed final : public Led {
public:
  StatusLed();
  ~StatusLed() override;

  StatusLed(const StatusLed &)            = delete;
  StatusLed &operator=(const StatusLed &) = delete;
  StatusLed(StatusLed &&)                 = delete;
  StatusLed &operator=(StatusLed &&)      = delete;

  void show(LedColor color, LedMode mode) override;
  void turnOff() override;

private:
  void applyColor(LedColor color);
  void clearColor();

  // The Pi's two built-in LEDs, by their kernel names: ACT is green, PWR is red.
  SysfsLed m_greenLed{"ACT", /*inverted=*/true};
  SysfsLed m_redLed{"PWR", /*inverted=*/false};
};

} // namespace warden::hardware
