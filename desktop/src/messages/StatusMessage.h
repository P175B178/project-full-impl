#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>

#include <expected>

/// Parses the status payloads on `sensors/<group-id>/status` (see protocol/examples/status-*.json).
/// Malformed input gives an error string; unknown JSON fields are ignored.
namespace warden::messages::status_message {

/// Device state, as in protocol/.
enum class State {
  Normal,  // no threshold exceeded
  Warning, // exactly one threshold exceeded
  Alert,   // both thresholds exceeded
  Fault,   // the sensor keeps failing
  Offline, // the device has disconnected
};

/// One parsed status message.
struct Status {
  QString deviceId;    // `deviceId` — the device's display name
  State state{};       // `state`
  QDateTime timestamp; // `timestamp`, always normalized to UTC
};

/// The status, or why the payload was rejected.
[[nodiscard]] std::expected<Status, QString> parse(const QByteArray &payload);

/// The state's name for display: "Normal", "Warning", ...
[[nodiscard]] QString stateName(State state);
} // namespace warden::messages::status_message
