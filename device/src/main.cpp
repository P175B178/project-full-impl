#include "sim/SimulatedSensor.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

int main() {
  constexpr auto readInterval = 5s;

  warden::sim::SimulatedSensor sensor;
  spdlog::info("Warden starting — simulated sensor");

  while (true) {
    const auto result = sensor.read();

    if (!result) {
      spdlog::warn("Sensor read failed ({}) — skipping reading", toString(result.error()));
    } else {
      spdlog::info("temp={:.1f} °C  hum={:.1f}%", result->temperature, result->humidity);
    }

    std::this_thread::sleep_for(readInterval);
  }
}
