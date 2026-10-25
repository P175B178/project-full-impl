#include "GpioBuzzer.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <ranges>
#include <thread>

using namespace std::chrono_literals;

namespace {
// Passive buzzer needs a square wave — tone frequency determines pitch.
constexpr int toneFreqHz = 2000;

constexpr auto tonePeriod = std::chrono::microseconds{1'000'000 / toneFreqHz};

// GPIO 17, physical pin 11 — fixed by hardware wiring.
constexpr unsigned int gpioPin = 17;

constexpr auto shortBeepDuration = 100ms;
constexpr auto shortBeepPause    = 100ms;
constexpr auto longBeepDuration  = 500ms;
} // namespace

namespace warden::hardware {

GpioBuzzer::GpioBuzzer(const std::string &chipPath)
      : m_pin{chipPath, gpioPin, "buzzer"} {}

// turnOff() is virtual, but GpioBuzzer is final, so this calls the version below.
GpioBuzzer::~GpioBuzzer() { turnOff(); }

void GpioBuzzer::turnOff() {
  spdlog::debug("GpioBuzzer: turnOff");
  m_pin.setLow();
}

void GpioBuzzer::shortBeep(unsigned int count) {
  spdlog::debug("GpioBuzzer: {} short beep(s)", count);
  beep(shortBeepDuration, count);
}

void GpioBuzzer::longBeep(unsigned int count) {
  spdlog::debug("GpioBuzzer: {} long beep(s)", count);
  beep(longBeepDuration, count);
}

void GpioBuzzer::beep(std::chrono::milliseconds duration, unsigned int count) {
  for (auto beepIdx : std::views::iota(0U, count)) {
    emitTone(duration);
    if (beepIdx + 1 < count) {
      std::this_thread::sleep_for(shortBeepPause);
    }
  }
}

void GpioBuzzer::emitTone(std::chrono::milliseconds duration) {
  const auto end = std::chrono::steady_clock::now() + duration;
  while (std::chrono::steady_clock::now() < end) {
    m_pin.setHigh();
    std::this_thread::sleep_for(tonePeriod / 2);
    m_pin.setLow();
    std::this_thread::sleep_for(tonePeriod / 2);
  }
  m_pin.setLow();
}

} // namespace warden::hardware
