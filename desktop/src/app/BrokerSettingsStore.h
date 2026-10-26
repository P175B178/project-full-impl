#pragma once

#include "mqtt/BrokerSettings.h"

/// Remembers the broker settings between runs, with `QSettings`. Not the password: QSettings
/// stores plain text.
namespace warden::app::broker_settings_store {

[[nodiscard]] mqtt::BrokerSettings load();
void save(const mqtt::BrokerSettings &settings);

} // namespace warden::app::broker_settings_store
