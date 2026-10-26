#include "core/DeviceState.h"

namespace warden::core {

namespace {

// Advance `latest` to `candidate` when the candidate is newer (or `latest` is still unset).
void advance(QDateTime &latest, const QDateTime &candidate) {
  if (!latest.isValid() || candidate > latest) {
    latest = candidate;
  }
}

} // namespace

void DeviceState::apply(const messages::observation::Reading &reading) {
  deviceId = reading.deviceId;
  switch (reading.metric) {
  case messages::observation::Metric::Temperature:
    temperature     = reading.value;
    temperatureUnit = reading.unit;
    break;
  case messages::observation::Metric::Humidity:
    humidity     = reading.value;
    humidityUnit = reading.unit;
    break;
  }
  advance(lastUpdate, reading.effectiveTime);
}

void DeviceState::apply(const messages::status_message::Status &status) {
  deviceId  = status.deviceId;
  state     = status.state;
  hasStatus = true;
  advance(lastUpdate, status.timestamp);
}

} // namespace warden::core
