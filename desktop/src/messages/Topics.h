#pragma once

#include <QString>

/// The topics of protocol/: what a device publishes, on `sensors/<group-id>/...`.
namespace warden::messages::topics {

[[nodiscard]] QString temperature(const QString &groupId);
[[nodiscard]] QString humidity(const QString &groupId);

} // namespace warden::messages::topics
