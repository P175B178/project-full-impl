#include "core/StateMachine.h"
#include "sim/ConsoleBuzzer.h"
#include "sim/ConsoleLed.h"
#include "sim/SimulatedSensor.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdlib>
#include <exception>
#include <thread>

// Everything except main() is internal to this file.
namespace {
using namespace std::chrono_literals;

// ── Entry point ──────────────────────────────────────────────────────────────
int runApp() {
  constexpr auto readInterval = 5s;

  warden::sim::SimulatedSensor sensor;
  warden::sim::ConsoleLed led;
  warden::sim::ConsoleBuzzer buzzer;
  warden::StateMachine stateMachine{warden::Thresholds{}};
  spdlog::info("Warden starting — simulated sensor");

  led.show(warden::LedColor::Green, warden::LedMode::Steady);

  while (true) {
    const auto result = sensor.read();

    if (!result) {
      spdlog::warn("Sensor read failed ({}) — skipping reading", toString(result.error()));
    } else {
      const auto transition = stateMachine.update(*result);

      spdlog::info("temp={:.1f} °C  hum={:.1f}%  state={}", result->temperature, result->humidity,
                   toString(stateMachine.state()));
      if (transition) {
        spdlog::warn("State change: {} -> {}", toString(transition->from), toString(transition->to));
        switch (transition->to) {
        case warden::State::Normal:
          led.show(warden::LedColor::Green, warden::LedMode::Steady);
          break;
        case warden::State::Warning:
          led.show(warden::LedColor::Orange, warden::LedMode::Steady);
          break;
        case warden::State::Alert:
          led.show(warden::LedColor::Red, warden::LedMode::Blinking);
          break;
        }
        if (transition->to == warden::State::Alert) {
          buzzer.shortBeep(3U);
        } else if (transition->from == warden::State::Alert) {
          buzzer.longBeep(1U);
        }
      }
    }

    std::this_thread::sleep_for(readInterval);
  }
}
} // namespace

int main() {
  // Anything that throws ends here, so the process logs the reason and exits instead of aborting.
  try {
    return runApp();
  } catch (const std::exception &err) {
    spdlog::critical("Fatal error: {}", err.what());
  } catch (...) {
    spdlog::critical("Fatal error: unknown exception");
  }
  return EXIT_FAILURE;
}
