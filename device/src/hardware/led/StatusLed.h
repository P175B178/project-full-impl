#pragma once

#include "interfaces/Led.h"

namespace warden::hardware {

class StatusLed final : public Led {
public:
  void show(LedColor color, LedMode mode) override;
  void turnOff() override;
};

} // namespace warden::hardware
