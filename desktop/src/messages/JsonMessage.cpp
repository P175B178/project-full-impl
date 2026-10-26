#include "messages/JsonMessage.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QLatin1Char>

namespace warden::messages::json_message {

std::expected<QJsonObject, QString> parseObject(const QByteArray &payload) {
  QJsonParseError parseError;
  const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    return std::unexpected(QStringLiteral("invalid JSON: %1").arg(parseError.errorString()));
  }
  if (!doc.isObject()) {
    return std::unexpected(QStringLiteral("payload is not a JSON object"));
  }
  return doc.object();
}

std::expected<QDateTime, QString> parseUtcTime(const QJsonObject &object, const QString &field) {
  const QString text = object.value(field).toString();
  if (!text.endsWith(QLatin1Char('Z'))) {
    return std::unexpected(QStringLiteral("%1 must be UTC and end with 'Z': \"%2\"").arg(field, text));
  }
  const QDateTime time = QDateTime::fromString(text, Qt::ISODate);
  if (!time.isValid()) {
    return std::unexpected(QStringLiteral("%1 is not valid ISO-8601: \"%2\"").arg(field, text));
  }
  return time.toUTC();
}

} // namespace warden::messages::json_message
