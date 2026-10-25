#include "core/AveragingBuffer.h"
#include "core/SignalTransition.h"
#include "core/StateMachine.h"
#include "sim/ConsoleBuzzer.h"
#include "sim/ConsoleLed.h"
#include "sim/SimulatedSensor.h"

#include <spdlog/spdlog.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <mutex>
#include <stop_token>
#include <thread>

// Everything except main() is internal to this file.
namespace {
using namespace std::chrono_literals;

// ── Signal handling ──────────────────────────────────────────────────────────

// A signal handler may only touch lock-free atomics, so it just records the request. The watcher
// thread below acts on it.
std::atomic<bool> stopSignalReceived{false}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static_assert(std::atomic<bool>::is_always_lock_free);

// The first signal also restores the default action, so a second Ctrl-C ends the program at once if
// the clean shutdown ever hangs.
extern "C" void onStopSignal(int signalNumber) {
  stopSignalReceived = true;
  std::signal(signalNumber, SIG_DFL);
}

/// Installs handlers for SIGINT (Ctrl+C), SIGTERM (kill) and SIGHUP (terminal or SSH connection
/// lost), and starts a thread that requests appStop when one arrives.
std::jthread watchForStopSignals(std::stop_source &appStop) {
  std::signal(SIGINT, onStopSignal);
  std::signal(SIGTERM, onStopSignal);
  std::signal(SIGHUP, onStopSignal);

  return std::jthread{[&appStop](const std::stop_token &stop) {
    while (!stop.stop_requested() && !stopSignalReceived) {
      std::this_thread::sleep_for(100ms);
    }
    if (stopSignalReceived) {
      spdlog::info("Stop signal received");
      appStop.request_stop();
    }
  }};
}

// ── Entry point ──────────────────────────────────────────────────────────────
int runApp() {
  constexpr auto readInterval           = 5s;
  constexpr std::size_t averagingWindow = 10;

  std::stop_source appStop;
  const auto signalWatcher = watchForStopSignals(appStop);
  const auto stopToken     = appStop.get_token();

  warden::sim::SimulatedSensor sensor;
  warden::sim::ConsoleLed led;
  warden::sim::ConsoleBuzzer buzzer;
  warden::StateMachine stateMachine{warden::Thresholds{}};
  warden::AveragingBuffer<float> temperatureBuffer{averagingWindow};
  warden::AveragingBuffer<float> humidityBuffer{averagingWindow};
  spdlog::info("Warden starting — simulated sensor");
  spdlog::info("Running — press Ctrl-C to stop");

  led.show(warden::LedColor::Green, warden::LedMode::Steady);

  // C++20 has no sleep_for() that takes a stop_token, but condition_variable_any::wait_for() does:
  // a stop request ends the wait. The predicate is never true, so nothing else does.
  std::mutex sleepMutex;
  std::condition_variable_any sleepCv;

  while (!stopToken.stop_requested()) {
    const auto result = sensor.read();

    if (!result) {
      spdlog::warn("Sensor read failed ({}) — skipping reading", toString(result.error()));
    } else {
      temperatureBuffer.push(result->temperature);
      humidityBuffer.push(result->humidity);

      // Never empty right after push(): value_or() only avoids an unchecked access.
      const warden::SmoothedReading smoothed{.temperature = temperatureBuffer.average().value_or(result->temperature),
                                             .humidity    = humidityBuffer.average().value_or(result->humidity)};

      const auto transition = stateMachine.update(smoothed);

      spdlog::info("temp={:.1f} °C (raw={:.1f})  hum={:.1f}% (raw={:.1f})  state={}", smoothed.temperature,
                   result->temperature, smoothed.humidity, result->humidity, toString(stateMachine.state()));
      if (transition) {
        spdlog::warn("State change: {} -> {}", toString(transition->from), toString(transition->to));
        warden::signalTransition(led, buzzer, *transition);
      }
    }

    // Sleeps after failed reads too, so broken hardware is not hammered.
    std::unique_lock lock{sleepMutex};
    sleepCv.wait_for(lock, stopToken, readInterval, [] { return false; });
  }

  led.turnOff();
  buzzer.turnOff();
  spdlog::info("Shutting down");
  return EXIT_SUCCESS;
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
