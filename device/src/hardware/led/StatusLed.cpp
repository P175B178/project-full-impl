#include "StatusLed.h"

#include <spdlog/spdlog.h>

#include <chrono>
#include <stop_token>
#include <thread>

using namespace std::chrono_literals;

namespace {
constexpr auto blinkPeriod = 1000ms;
}

namespace warden::hardware {

StatusLed::StatusLed() = default;

// turnOff() is virtual, but StatusLed is final, so this calls the version below.
StatusLed::~StatusLed() { turnOff(); }

void StatusLed::clearColor() {
  m_greenLed.turnOff();
  m_redLed.turnOff();
}

void StatusLed::applyColor(LedColor color) {
  switch (color) {
  case LedColor::Green:
    m_greenLed.turnOn();
    m_redLed.turnOff();
    break;
  case LedColor::Orange:
    m_greenLed.turnOn();
    m_redLed.turnOn();
    break;
  case LedColor::Red:
    m_greenLed.turnOff();
    m_redLed.turnOn();
    break;
  }
}

void StatusLed::show(LedColor color, LedMode mode) {
  spdlog::debug("StatusLed: show color={} blink={}", toString(color), mode == LedMode::Blinking);
  // Stop the blink thread first, or it could switch the LEDs after the color is set below.
  stopBlinking();
  if (mode == LedMode::Blinking) {
    startBlinking(color);
  } else {
    applyColor(color);
  }
}

void StatusLed::turnOff() {
  stopBlinking();
  clearColor();
}

void StatusLed::startBlinking(LedColor color) {
  m_blinkThread = std::jthread{[this, color](const std::stop_token &stop) {
    bool lit = true;

    spdlog::debug("blink thread started");
    while (!stop.stop_requested()) {
      spdlog::debug("blink: {}", lit ? "on" : "off");

      if (lit) {
        applyColor(color);
      } else {
        clearColor();
      }

      // A plain sleep: stopping waits for at most the rest of this half period, 0.5 s.
      std::this_thread::sleep_for(blinkPeriod / 2);

      lit = !lit;
    }

    clearColor();
    spdlog::debug("blink thread stopped");
  }};
}

void StatusLed::stopBlinking() {
  if (m_blinkThread.joinable()) {
    spdlog::debug("StatusLed: stopping blink thread");
    m_blinkThread.request_stop();
    m_blinkThread.join();
  }
}

} // namespace warden::hardware
