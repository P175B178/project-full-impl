#pragma once

#include "GpioPin.h"
#include "interfaces/Buzzer.h"

#include <chrono>
#include <string>

namespace warden::hardware {

/// Passive buzzer driven by a single GPIO output pin. Emits a tone by toggling the pin at a fixed
/// frequency.
///
/// shortBeep() and longBeep() block until the beeps have played.
class GpioBuzzer final : public Buzzer {
public:
  explicit GpioBuzzer(const std::string &chipPath = "/dev/gpiochip4");
  ~GpioBuzzer() override;

  GpioBuzzer(const GpioBuzzer &)            = delete;
  GpioBuzzer &operator=(const GpioBuzzer &) = delete;
  GpioBuzzer(GpioBuzzer &&)                 = delete;
  GpioBuzzer &operator=(GpioBuzzer &&)      = delete;

  void shortBeep(unsigned int count) override;
  void longBeep(unsigned int count) override;
  void turnOff() override;

private:
  void beep(std::chrono::milliseconds duration, unsigned int count);
  void emitTone(std::chrono::milliseconds duration);

  GpioPin m_pin;
};

} // namespace warden::hardware
