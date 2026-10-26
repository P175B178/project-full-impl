#include "app/BrokerSettingsStore.h"

#include <QSettings>

namespace warden::app::broker_settings_store {

namespace {

constexpr auto hostKey     = "broker/host";
constexpr auto portKey     = "broker/port";
constexpr auto usernameKey = "broker/username";
constexpr auto useTlsKey   = "broker/useTls";
constexpr auto caCertKey   = "broker/caCertPath";

} // namespace

mqtt::BrokerSettings load() {
  const QSettings settings;
  const mqtt::BrokerSettings defaults;
  return mqtt::BrokerSettings{
      .host       = settings.value(hostKey, defaults.host).toString(),
      .port       = static_cast<quint16>(settings.value(portKey, defaults.port).toUInt()),
      .username   = settings.value(usernameKey, defaults.username).toString(),
      .password   = QString(), // never stored
      .useTls     = settings.value(useTlsKey, defaults.useTls).toBool(),
      .caCertPath = settings.value(caCertKey, defaults.caCertPath).toString(),
  };
}

void save(const mqtt::BrokerSettings &settings) {
  QSettings store;
  store.setValue(hostKey, settings.host);
  store.setValue(portKey, settings.port);
  store.setValue(usernameKey, settings.username);
  store.setValue(useTlsKey, settings.useTls);
  store.setValue(caCertKey, settings.caCertPath);
}

} // namespace warden::app::broker_settings_store
