#pragma once

#include "messages/StatusMessage.h"

#include <QColor>

#include <optional>

namespace warden::app {

// The console's colors, kept in one place. Each reads on a light window and on a dark one.

/// The color a device's state is shown in, wherever the console shows one: green Normal, orange
/// Warning, red Alert, purple Fault (the sensor, not the room, needs attention), and gray for
/// Offline or a state not known yet (nullopt).
[[nodiscard]] QColor stateColor(std::optional<messages::status_message::State> state);

/// A history cell above its threshold: a translucent red, which tints a light table and a dark one
/// alike and keeps the text readable.
[[nodiscard]] QColor violationBackground();

/// A device's line on the charts, by the order the devices were found in; after six, the colors
/// repeat.
[[nodiscard]] QColor seriesColor(qsizetype index);

/// The hint that the charts are paused: dark text on a light amber band of its own.
[[nodiscard]] QColor hintText();
[[nodiscard]] QColor hintBackground();

} // namespace warden::app
