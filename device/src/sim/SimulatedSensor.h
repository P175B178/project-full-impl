#pragma once

#include "interfaces/Reading.h"
#include "interfaces/Sensor.h"

#include <chrono>
#include <cmath>
#include <numbers>
#include <random>

namespace warden::sim {

/// Simulated sensor: a slow sine wave, repeating every 3 minutes from startup. Occasionally
/// misbehaves (see read()).
class SimulatedSensor final : public Sensor {
public:
  [[nodiscard]] std::expected<Reading, SensorError> read() override {
    using namespace std::chrono;

    // NOLINTBEGIN(readability-magic-numbers)
    constexpr auto cyclePeriod = minutes{3};

    const double phase = duration<double>(steady_clock::now() - m_start) / cyclePeriod * (2.0 * std::numbers::pi);

    // Humidity lags temperature slightly, so the two rise and fall one after the other.
    Reading reading;
    reading.temperature = 24.0F + 8.0F * static_cast<float>(std::sin(phase));        // 16-32 C
    reading.humidity    = 60.0F + 15.0F * static_cast<float>(std::sin(phase - 0.5)); // 45-75 %
    reading.timestamp   = system_clock::now();

    // Each case has a 1 in 20 chance (~10% in total):
    switch (m_faultDist(m_rng)) {
    case 0:
      return std::unexpected{SensorError::InvalidData};
    case 1:
      // Valid for a DHT22, but implausible for a room.
      reading.temperature = 70.0F;
      break;
    default:
      break;
    }
    // NOLINTEND(readability-magic-numbers)

    return reading;
  }

private:
  std::chrono::steady_clock::time_point m_start{std::chrono::steady_clock::now()};
  std::mt19937 m_rng{std::random_device{}()};
  std::uniform_int_distribution<int> m_faultDist{0, 19}; // NOLINT(readability-magic-numbers)
};

} // namespace warden::sim
