#pragma once

#include "messages/Observation.h"

#include <QByteArray>
#include <QString>

#include <expected>

/// Tells what a message on the devices' topics is, from its topic (protocol/), and parses it.
namespace warden::messages::message_router {

/// A reading on `sensors/<group-id>/temperature` or `.../humidity`.
struct Reading {
  QString groupId;
  observation::Reading reading;
};

/// The reading on `topic`, or why it cannot be used: an unexpected topic, a payload the parser
/// rejects, or a reading whose code does not match its topic.
[[nodiscard]] std::expected<Reading, QString> route(const QString &topic, const QByteArray &payload);

} // namespace warden::messages::message_router
