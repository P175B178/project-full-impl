#include "app/Colors.h"

#include <QString>

namespace warden::app {

QColor stateColor(std::optional<messages::status_message::State> state) {
  const QColor gray(QStringLiteral("#9e9e9e"));
  if (!state.has_value()) {
    return gray;
  }
  switch (state.value()) {
  case messages::status_message::State::Normal:
    return {QStringLiteral("#4caf50")}; // green
  case messages::status_message::State::Warning:
    return {QStringLiteral("#ff9800")}; // orange
  case messages::status_message::State::Alert:
    return {QStringLiteral("#f44336")}; // red
  case messages::status_message::State::Fault:
    return {QStringLiteral("#9c27b0")}; // purple
  case messages::status_message::State::Offline:
    return gray;
  }
  return gray;
}

QColor violationBackground() {
  constexpr int alpha = 90; // about a third opaque
  QColor red(QStringLiteral("#f44336"));
  red.setAlpha(alpha);
  return red;
}

} // namespace warden::app
