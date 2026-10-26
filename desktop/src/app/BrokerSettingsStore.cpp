#include "app/BrokerSettingsStore.h"

#include <QSettings>

namespace warden::app::broker_settings_store {

namespace {

constexpr auto hostKey = "broker/host";
constexpr auto portKey = "broker/port";

} // namespace

mqtt::BrokerSettings load() {
  const QSettings settings;
  const mqtt::BrokerSettings defaults;
  return mqtt::BrokerSettings{
      .host = settings.value(hostKey, defaults.host).toString(),
      .port = static_cast<quint16>(settings.value(portKey, defaults.port).toUInt()),
  };
}

void save(const mqtt::BrokerSettings &settings) {
  QSettings store;
  store.setValue(hostKey, settings.host);
  store.setValue(portKey, settings.port);
}

} // namespace warden::app::broker_settings_store
