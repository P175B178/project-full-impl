#include "Dht22Sensor.h"

namespace warden::hardware {

std::expected<Reading, SensorError> Dht22Sensor::read() {
  return std::unexpected{SensorError::ReadFailure}; // not implemented yet
}

} // namespace warden::hardware
