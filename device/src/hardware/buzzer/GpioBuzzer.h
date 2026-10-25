#pragma once

#include "interfaces/Buzzer.h"

namespace warden::hardware {

class GpioBuzzer final : public Buzzer {
public:
  void shortBeep(unsigned int count) override;
  void longBeep(unsigned int count) override;
  void turnOff() override;
};

} // namespace warden::hardware
