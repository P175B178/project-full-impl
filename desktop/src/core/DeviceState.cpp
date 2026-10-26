#include "core/DeviceState.h"

namespace warden::core {

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
}

void DeviceState::apply(const messages::status_message::Status &status) {
  deviceId  = status.deviceId;
  state     = status.state;
  hasStatus = true;
}

} // namespace warden::core
