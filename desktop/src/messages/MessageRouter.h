#pragma once

#include "messages/Observation.h"
#include "messages/StatusMessage.h"

#include <QByteArray>
#include <QString>

#include <expected>
#include <optional>
#include <variant>

/// Tells what a message on the devices' topics is, from its topic (protocol/), and parses it.
namespace warden::messages::message_router {

/// A reading on `sensors/<group-id>/temperature` or `.../humidity`.
struct Reading {
  QString groupId;
  observation::Reading reading;
};

/// A status on `sensors/<group-id>/status`.
struct Status {
  QString groupId;
  status_message::Status status;
};

/// A setting a device was last sent, on `config/<group-id>/...`; the value is nullopt when the
/// setting was cleared: an empty retained message removes it from the broker.
struct TemperatureThreshold {
  QString groupId;
  std::optional<double> celsius;
};
struct HumidityThreshold {
  QString groupId;
  std::optional<double> percent;
};
struct Interval {
  QString groupId;
  std::optional<int> seconds;
};

using Message = std::variant<Reading, Status, TemperatureThreshold, HumidityThreshold, Interval>;

/// The message on `topic`, or why it cannot be used: an unexpected topic, a payload its parser
/// rejects, or a reading whose code does not match its topic.
[[nodiscard]] std::expected<Message, QString> route(const QString &topic, const QByteArray &payload);

} // namespace warden::messages::message_router
