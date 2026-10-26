#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QJsonObject>
#include <QString>

#include <expected>

/// What the parsers of the protocol's JSON messages share.
namespace warden::messages::json_message {

/// The payload as a JSON object, or what is wrong with it.
[[nodiscard]] std::expected<QJsonObject, QString> parseObject(const QByteArray &payload);

/// The time in `object[field]`, in UTC, or what is wrong with it. UTC only: a `Z` suffix; other
/// offsets are rejected (protocol/).
[[nodiscard]] std::expected<QDateTime, QString> parseUtcTime(const QJsonObject &object, const QString &field);

} // namespace warden::messages::json_message
