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

} // namespace warden::app
