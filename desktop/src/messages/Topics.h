#pragma once

#include <QString>

/// The topics of protocol/: what a device publishes, on `sensors/<group-id>/...`, and what
/// it is sent, on `config/<group-id>/...`.
namespace warden::messages::topics {

[[nodiscard]] QString temperature(const QString &groupId);
[[nodiscard]] QString humidity(const QString &groupId);
[[nodiscard]] QString status(const QString &groupId);
[[nodiscard]] QString temperatureThreshold(const QString &groupId);
[[nodiscard]] QString humidityThreshold(const QString &groupId);
[[nodiscard]] QString interval(const QString &groupId);
[[nodiscard]] QString forcedState(const QString &groupId);

} // namespace warden::messages::topics
