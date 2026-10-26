#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>

#include <expected>
#include <optional>

/// Parses the FHIR Observation payloads on `sensors/<group-id>/temperature` and `.../humidity`
/// (see protocol/examples/observation-*.json). Malformed input gives an error string; unknown JSON
/// fields are ignored.
namespace warden::messages::observation {

/// Which metric a reading carries, selected by the LOINC code in `code.code`.
enum class Metric {
  Temperature, // LOINC 8310-5
  Humidity,    // LOINC 13834-7
};

/// One parsed measurement.
struct Reading {
  QString deviceId;        // FHIR `deviceId` — the device's display name
  Metric metric{};         // selected from the LOINC code
  double value{};          // `valueQuantity.value`
  QString unit;            // `valueQuantity.unit`, e.g. "°C" or "%"
  QDateTime effectiveTime; // `effectiveDateTime`, always normalized to UTC
};

/// The reading, or why the payload was rejected.
[[nodiscard]] std::expected<Reading, QString> parse(const QByteArray &payload);

/// A value for display, "23.4 °C", or an em dash when there is none yet.
[[nodiscard]] QString format(const std::optional<double> &value, const QString &unit);
} // namespace warden::messages::observation
