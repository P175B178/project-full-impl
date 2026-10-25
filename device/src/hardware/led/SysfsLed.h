#pragma once

#include <string>

namespace warden::hardware {

/// Controls a single Linux sysfs LED (/sys/class/leds/<name>/).
///
/// Takes over the LED by setting its kernel trigger to "none". On destruction it turns the LED off,
/// and it stays off until the next reboot.
///
/// The inverted flag is for the Pi 5 ACT LED, where brightness 0 means on.
///
/// The constructor throws std::runtime_error if the LED cannot be written
/// (tools/pi/init/02-bootstrap.sh sets up the permission).
class SysfsLed {
public:
  /// @param name     LED name as it appears under /sys/class/leds/, e.g. "ACT"
  /// @param inverted true if 0=on, 1=off (e.g. Pi 5 ACT LED)
  explicit SysfsLed(const std::string &name, bool inverted = false);
  ~SysfsLed();

  SysfsLed(const SysfsLed &)            = delete;
  SysfsLed &operator=(const SysfsLed &) = delete;
  SysfsLed(SysfsLed &&)                 = delete;
  SysfsLed &operator=(SysfsLed &&)      = delete;

  void turnOn() noexcept;
  void turnOff() noexcept;

private:
  void setBrightness(const char *value) noexcept;
  /// Writes `value` to the file at `path`. Returns false if that fails.
  [[nodiscard]] static bool writeFile(const std::string &path, const char *value) noexcept;

  std::string m_basePath;
  bool m_inverted;
};

} // namespace warden::hardware
