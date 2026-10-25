#include "core/Config.h"
#include "core/WardenApp.h"
#include "sim/ConsoleBuzzer.h"
#include "sim/ConsoleLed.h"
#include "sim/SimulatedSensor.h"

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <expected>
#include <stop_token>
#include <string>
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

// ── Command line ─────────────────────────────────────────────────────────────
struct CommandLine {
  std::string configPath{"/etc/warden/warden.toml"};
};

/// Returns the arguments, or the exit code to stop with: 0 after --help or --version, non-zero for
/// bad arguments.
std::expected<CommandLine, int> parseCommandLine(int argc, char **argv) {
  CommandLine commandLine;

  CLI::App app{"Warden — environmental monitor", "warden"};
  app.set_version_flag("--version", std::string{WARDEN_VERSION});
  app.add_option("-c,--config", commandLine.configPath, "Config file")->capture_default_str();

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &err) {
    return std::unexpected{app.exit(err)}; // prints the help, version or error
  }
  return commandLine;
}

// ── Monitoring ───────────────────────────────────────────────────────────────
/// Constructs the sensor, LED and buzzer and runs the monitoring loop until stopToken is signalled.
void runMonitoring(const warden::Config &config, const std::stop_token &stopToken) {
  warden::sim::SimulatedSensor sensor;
  warden::sim::ConsoleLed led;
  warden::sim::ConsoleBuzzer buzzer;

  spdlog::info("Running — press Ctrl-C to stop");

  warden::WardenApp app{sensor, led, buzzer, config};
  app.run(stopToken);
}

// ── Entry point ──────────────────────────────────────────────────────────────
int runApp(int argc, char **argv) {
  const auto commandLine = parseCommandLine(argc, argv);
  if (!commandLine) {
    return commandLine.error();
  }

  std::stop_source appStop;
  const auto signalWatcher = watchForStopSignals(appStop);

  const auto configResult = warden::loadConfig(commandLine->configPath);
  if (!configResult) {
    spdlog::error("Failed to load config '{}': {}", commandLine->configPath, configResult.error());
    return EXIT_FAILURE;
  }
  const auto &config = *configResult;

  spdlog::info("Warden {} starting — config: {}", WARDEN_VERSION, commandLine->configPath);
  spdlog::info("Temperature threshold: {:.1f} °C, humidity threshold: {:.1f}%", config.thresholds.temperature,
               config.thresholds.humidity);
  spdlog::info("Read interval: {}s, averaging window: {} readings", config.readInterval.count(),
               config.averagingWindow);
  spdlog::info("Sensor fault after {} failed reads in a row", config.faultAfterFailedReads);
  spdlog::info("Plausible range: temp [{:.1f}, {:.1f}] °C  hum [{:.1f}, {:.1f}]%",
               config.plausibleValues.temperature.min, config.plausibleValues.temperature.max,
               config.plausibleValues.humidity.min, config.plausibleValues.humidity.max);

  runMonitoring(config, appStop.get_token());

  spdlog::info("Shutting down");
  return EXIT_SUCCESS;
}
} // namespace

int main(int argc, char **argv) {
  // Anything that throws ends here, so the process logs the reason and exits instead of aborting.
  try {
    return runApp(argc, argv);
  } catch (const std::exception &err) {
    spdlog::critical("Fatal error: {}", err.what());
  } catch (...) {
    spdlog::critical("Fatal error: unknown exception");
  }
  return EXIT_FAILURE;
}
