#pragma once

#include "GpioPin.h"
#include "interfaces/Buzzer.h"

#include <chrono>
#include <stop_token>
#include <string>
#include <thread>

namespace warden::hardware {

/// Passive buzzer driven by a single GPIO output pin. Emits a tone by toggling the pin at a fixed
/// frequency.
///
/// shortBeep() and longBeep() play on a background thread and return at once. A new call, or
/// turnOff(), stops a beep still playing.
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
  void startBeeping(std::chrono::milliseconds duration, unsigned int count);
  void stopBeeping();
  void emitTone(std::chrono::milliseconds duration, const std::stop_token &stop);

  GpioPin m_pin;
  std::jthread m_beepThread;
};

} // namespace warden::hardware
