#pragma once

#include <gpiod.h>
#include <memory>
#include <string>

namespace warden::hardware {

/// RAII wrapper around a single libgpiod v2 output line.
///
/// The chip path on Raspberry Pi 5 is typically /dev/gpiochip4 (RP1). Pin numbers use BCM
/// numbering.
class GpioPin {
public:
  /// @param chipPath   e.g. "/dev/gpiochip4"
  /// @param lineOffset BCM pin number
  /// @param consumer   label shown in gpioinfo
  GpioPin(const std::string &chipPath, unsigned int lineOffset, const std::string &consumer);
  ~GpioPin() = default;

  GpioPin(const GpioPin &)            = delete;
  GpioPin &operator=(const GpioPin &) = delete;
  GpioPin(GpioPin &&)                 = delete;
  GpioPin &operator=(GpioPin &&)      = delete;

  void setHigh();
  void setLow();

private:
  struct ChipDeleter {
    void operator()(gpiod_chip *ptr) const noexcept { gpiod_chip_close(ptr); }
  };
  struct RequestDeleter {
    void operator()(gpiod_line_request *ptr) const noexcept { gpiod_line_request_release(ptr); }
  };

  // Declared before `request` so the line is released before the chip closes.
  std::unique_ptr<gpiod_chip, ChipDeleter> m_chip;
  std::unique_ptr<gpiod_line_request, RequestDeleter> m_request;
  unsigned int m_offset{0};
};

} // namespace warden::hardware
