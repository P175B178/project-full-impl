#include "SysfsLed.h"

#include <spdlog/spdlog.h>

#include <format>
#include <fstream>
#include <stdexcept>
#include <string>

namespace warden::hardware {

bool SysfsLed::writeFile(const std::string &path, const char *value) noexcept {
  std::ofstream file{path};
  file << value << std::flush;
  return file.good();
}

SysfsLed::SysfsLed(const std::string &name, bool inverted)
      : m_basePath{"/sys/class/leds/" + name}
      , m_inverted{inverted} {
  spdlog::debug("SysfsLed: init '{}' (inverted={})", name, inverted);
  // Without the LED warden cannot show its state, so stop at startup.
  if (!writeFile(m_basePath + "/trigger", "none") || !writeFile(m_basePath + "/brightness", inverted ? "1" : "0")) {
    throw std::runtime_error{
        std::format("Cannot write to {}: the LED is missing, or this user may not write to it", m_basePath)};
  }
}

SysfsLed::~SysfsLed() { turnOff(); }

void SysfsLed::turnOn() noexcept { setBrightness(m_inverted ? "0" : "1"); }

void SysfsLed::turnOff() noexcept { setBrightness(m_inverted ? "1" : "0"); }

void SysfsLed::setBrightness(const char *value) noexcept {
  if (!writeFile(m_basePath + "/brightness", value)) {
    spdlog::warn("SysfsLed: cannot write to {}/brightness", m_basePath);
  }
}

} // namespace warden::hardware
