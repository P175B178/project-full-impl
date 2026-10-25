#pragma once

#include "interfaces/Sensor.h"

#include <string>

namespace warden::hardware {

/// Reads temperature and humidity from a DHT22 via the Linux kernel IIO driver.
///
/// Setup (add to /boot/firmware/config.txt, then reboot):
///   dtoverlay=dht11,gpiopin=4
///   (The dht11 overlay name is correct — it supports both DHT11 and DHT22.)
///
/// After loading, the kernel exposes readings via sysfs:
///   /sys/bus/iio/devices/iio:device0/in_temp_input          (millidegrees C)
///   /sys/bus/iio/devices/iio:device0/in_humidityrelative_input  (0.001 %)
///
/// The constructor throws std::runtime_error if that device does not exist.
class Dht22Sensor final : public Sensor {
public:
  explicit Dht22Sensor(std::string sysfsBasePath = "/sys/bus/iio/devices/iio:device0");

  [[nodiscard]] std::expected<Reading, SensorError> read() override;

private:
  [[nodiscard]] static std::expected<float, SensorError> readSysfsValue(const std::string &path, float scale);

  std::string m_sysfsBasePath;
};

} // namespace warden::hardware
