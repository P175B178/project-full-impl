#pragma once

#include "interfaces/Reading.h"
#include "interfaces/Sensor.h"

#include <chrono>
#include <cmath>
#include <numbers>
#include <random>

namespace warden::sim {

/// Simulated sensor: a slow sine wave, repeating every 3 minutes from startup. Occasionally
/// misbehaves (see read()). Once per cycle the sensor is lost for a while, to show a fault.
class SimulatedSensor final : public Sensor {
public:
  [[nodiscard]] std::expected<Reading, SensorError> read() override {
    using namespace std::chrono;

    // NOLINTBEGIN(readability-magic-numbers)
    constexpr auto cyclePeriod = minutes{3};

    constexpr auto sensorLostAt   = seconds{85}; // into each cycle, during the alarm
    constexpr int sensorLostReads = 10;          // the default config sees a fault after 5

    const auto elapsed = steady_clock::now() - m_start;
    const auto cycle   = elapsed / cyclePeriod; // number of whole cycles so far
    if (elapsed % cyclePeriod >= sensorLostAt && cycle != m_lastLostCycle) {
      m_lastLostCycle = cycle;
      m_lostReadsLeft = sensorLostReads;
    }
    if (m_lostReadsLeft > 0) {
      --m_lostReadsLeft;
      return std::unexpected{SensorError::ReadFailure};
    }

    const double phase = duration<double>(elapsed) / cyclePeriod * (2.0 * std::numbers::pi);

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
  std::chrono::steady_clock::rep m_lastLostCycle{-1};    ///< Cycle in which the sensor was last lost
  int m_lostReadsLeft{0};                                ///< Reads still to fail
};

} // namespace warden::sim
