#include "sim/ConsoleBuzzer.h"
#include "sim/ConsoleLed.h"
#include "sim/SimulatedSensor.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

int main() {
  constexpr auto readInterval          = 5s;
  constexpr float temperatureThreshold = 28.0F;
  constexpr float humidityThreshold    = 70.0F;

  warden::sim::SimulatedSensor sensor;
  warden::sim::ConsoleLed led;
  warden::sim::ConsoleBuzzer buzzer;
  spdlog::info("Warden starting — simulated sensor");

  // Green while both values are below their thresholds, orange when one is above, red (blinking,
  // with beeps) when both are.
  auto color = warden::LedColor::Green;
  led.show(color, warden::LedMode::Steady);

  while (true) {
    const auto result = sensor.read();

    if (!result) {
      spdlog::warn("Sensor read failed ({}) — skipping reading", toString(result.error()));
    } else {
      spdlog::info("temp={:.1f} °C  hum={:.1f}%", result->temperature, result->humidity);

      const bool hot   = result->temperature > temperatureThreshold;
      const bool humid = result->humidity > humidityThreshold;
      auto newColor    = warden::LedColor::Green;
      if (hot && humid) {
        newColor = warden::LedColor::Red;
      } else if (hot || humid) {
        newColor = warden::LedColor::Orange;
      }

      if (newColor != color) {
        if (newColor == warden::LedColor::Red) {
          led.show(newColor, warden::LedMode::Blinking);
          buzzer.shortBeep(3U);
        } else {
          led.show(newColor, warden::LedMode::Steady);
          if (color == warden::LedColor::Red) {
            buzzer.longBeep(1U);
          }
        }
        color = newColor;
      }
    }

    std::this_thread::sleep_for(readInterval);
  }
}
