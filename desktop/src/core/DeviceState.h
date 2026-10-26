#pragma once

#include "messages/Observation.h"

#include <QString>

#include <optional>

namespace warden::core {

/// The live state of one discovered device. `groupId`, from the topic, is the stable key;
/// `deviceId` is the display name the payloads carry.
struct DeviceState {
  QString groupId;  // from the topic: the stable key
  QString deviceId; // the name the messages carry

  std::optional<double> temperature; // latest temperature reading, if any
  QString temperatureUnit;

  std::optional<double> humidity; // latest humidity reading, if any
  QString humidityUnit;

  /// The name to show: the display name, once a message has given one.
  [[nodiscard]] QString displayName() const { return deviceId.isEmpty() ? groupId : deviceId; }

  /// Fold one parsed reading into the state, updating the metric it carries.
  void apply(const messages::observation::Reading &reading);
};

} // namespace warden::core
