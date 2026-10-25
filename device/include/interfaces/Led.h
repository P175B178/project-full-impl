#pragma once

namespace warden {

enum class LedColor { Green, Orange, Red };

/// An enum rather than a bool, so a call reads led.show(color, LedMode::Blinking).
enum class LedMode { Steady, Blinking };

/// Abstract LED interface.
class Led {
public:
  Led()                       = default;
  virtual ~Led()              = default;
  Led(const Led &)            = delete;
  Led &operator=(const Led &) = delete;
  Led(Led &&)                 = delete;
  Led &operator=(Led &&)      = delete;

  virtual void show(LedColor color, LedMode mode) = 0;
  virtual void turnOff()                          = 0;
};

} // namespace warden
