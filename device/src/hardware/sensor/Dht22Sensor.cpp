#include "Dht22Sensor.h"

#include "Range.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
// The DHT22 (AM2302) datasheet's range: a value outside it means a faulty sensor or driver.
constexpr warden::Range datasheetTemperature{.min = -40.0F, .max = 80.0F};
constexpr warden::Range datasheetHumidity{.min = 0.0F, .max = 100.0F};
} // namespace

namespace warden::hardware {

Dht22Sensor::Dht22Sensor(std::string sysfsBasePath)
      : m_sysfsBasePath{std::move(sysfsBasePath)} {
  // Without the kernel driver there is nothing to read, so stop at startup.
  if (!std::filesystem::exists(this->m_sysfsBasePath)) {
    throw std::runtime_error{std::format("No DHT22 at {}: is dtoverlay=dht11,gpiopin=4 in /boot/firmware/config.txt?",
                                         this->m_sysfsBasePath)};
  }
}

std::expected<Reading, SensorError> Dht22Sensor::read() {
  // Kernel IIO driver returns temperature in millidegrees C (scale: 0.001)
  const auto temperatureResult = readSysfsValue(m_sysfsBasePath + "/in_temp_input", 0.001F);
  if (!temperatureResult) {
    return std::unexpected{temperatureResult.error()};
  }

  // Humidity in 0.001 % (scale: 0.001)
  const auto humidityResult = readSysfsValue(m_sysfsBasePath + "/in_humidityrelative_input", 0.001F);
  if (!humidityResult) {
    return std::unexpected{humidityResult.error()};
  }

  const float temperature = *temperatureResult;
  const float humidity    = *humidityResult;

  if (!datasheetTemperature.contains(temperature) || !datasheetHumidity.contains(humidity)) {
    return std::unexpected{SensorError::InvalidData};
  }

  return Reading{.temperature = temperature, .humidity = humidity, .timestamp = std::chrono::system_clock::now()};
}

std::expected<float, SensorError> Dht22Sensor::readSysfsValue(const std::string &path, float scale) {
  std::ifstream file{path};
  if (!file.is_open()) {
    return std::unexpected{SensorError::ReadFailure};
  }

  int raw{};
  if (!(file >> raw)) {
    return std::unexpected{SensorError::ReadFailure};
  }

  const float value = static_cast<float>(raw) * scale;
  spdlog::debug("Dht22Sensor: raw={} scale={} value={:.3f} path={}", raw, scale, value, path);
  return value;
}

} // namespace warden::hardware
