#pragma once

#include "Reading.h"

#include <expected>

enum class SensorError {
  ReadFailure, ///< Could not read from the hardware (I/O error, device absent)
  InvalidData  ///< Reading outside the sensor's physical limits (hardware/driver error)
};

/// Abstract sensor interface.
///
/// Read errors are std::expected values, not exceptions: a failed read is expected and recoverable.
class Sensor {
public:
  [[nodiscard]] virtual std::expected<Reading, SensorError> read() = 0;
};
