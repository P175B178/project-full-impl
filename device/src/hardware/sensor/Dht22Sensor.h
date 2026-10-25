#pragma once

#include "interfaces/Sensor.h"

namespace warden::hardware {

class Dht22Sensor final : public Sensor {
public:
  [[nodiscard]] std::expected<Reading, SensorError> read() override;
};

} // namespace warden::hardware
