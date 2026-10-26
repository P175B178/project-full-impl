#include "messages/Observation.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QString>

namespace warden::messages::observation {

namespace {

constexpr QLatin1String loincTemperature{"8310-5"};
constexpr QLatin1String loincHumidity{"13834-7"};

constexpr int valuePrecision = 1;

} // namespace

std::expected<Reading, QString> parse(const QByteArray &payload) {
  QJsonParseError parseError;
  const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    return std::unexpected(QStringLiteral("invalid JSON: %1").arg(parseError.errorString()));
  }
  if (!doc.isObject()) {
    return std::unexpected(QStringLiteral("payload is not a JSON object"));
  }
  const QJsonObject root = doc.object();

  if (root.value("resourceType").toString() != QLatin1String("Observation")) {
    return std::unexpected(QStringLiteral("resourceType is not \"Observation\""));
  }

  const QString deviceId = root.value("deviceId").toString();
  if (deviceId.isEmpty()) {
    return std::unexpected(QStringLiteral("missing required field: deviceId"));
  }

  // `status` must be present; its value is not constrained at the parsing boundary.
  if (!root.value("status").isString()) {
    return std::unexpected(QStringLiteral("missing required field: status"));
  }

  // UTC only: a `Z` suffix; other offsets are rejected (protocol/).
  const QString effectiveText = root.value("effectiveDateTime").toString();
  if (!effectiveText.endsWith(QLatin1Char('Z'))) {
    return std::unexpected(QStringLiteral("effectiveDateTime must be UTC and end with 'Z': \"%1\"").arg(effectiveText));
  }
  const QDateTime effectiveTime = QDateTime::fromString(effectiveText, Qt::ISODate);
  if (!effectiveTime.isValid()) {
    return std::unexpected(QStringLiteral("effectiveDateTime is not valid ISO-8601: \"%1\"").arg(effectiveText));
  }

  const QJsonValue codeValue = root.value("code");
  if (!codeValue.isObject()) {
    return std::unexpected(QStringLiteral("missing required field: code"));
  }
  const QString loinc = codeValue.toObject().value("code").toString();
  Metric metric{};
  if (loinc == loincTemperature) {
    metric = Metric::Temperature;
  } else if (loinc == loincHumidity) {
    metric = Metric::Humidity;
  } else {
    return std::unexpected(QStringLiteral("unknown or missing LOINC code: \"%1\"").arg(loinc));
  }

  const QJsonValue quantityValue = root.value("valueQuantity");
  if (!quantityValue.isObject()) {
    return std::unexpected(QStringLiteral("missing required field: valueQuantity"));
  }
  const QJsonObject quantity    = quantityValue.toObject();
  const QJsonValue numericValue = quantity.value("value");
  if (!numericValue.isDouble()) {
    return std::unexpected(QStringLiteral("valueQuantity.value is missing or not a number"));
  }

  return Reading{
      .deviceId      = deviceId,
      .metric        = metric,
      .value         = numericValue.toDouble(),
      .unit          = quantity.value("unit").toString(),
      .effectiveTime = effectiveTime.toUTC(),
  };
}

QString format(const std::optional<double> &value, const QString &unit) {
  if (!value.has_value()) {
    return QStringLiteral("—");
  }
  const QString number = QString::number(value.value(), 'f', valuePrecision);
  return unit.isEmpty() ? number : QStringLiteral("%1 %2").arg(number, unit);
}

} // namespace warden::messages::observation
