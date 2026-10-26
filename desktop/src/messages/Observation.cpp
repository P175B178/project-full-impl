#include "messages/Observation.h"
#include "messages/JsonMessage.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace warden::messages::observation {

namespace {

constexpr QLatin1String loincTemperature{"8310-5"};
constexpr QLatin1String loincHumidity{"13834-7"};

constexpr int valuePrecision = 1;

} // namespace

std::expected<Reading, QString> parse(const QByteArray &payload) {
  const auto root = json_message::parseObject(payload);
  if (!root.has_value()) {
    return std::unexpected(root.error());
  }

  if (root->value("resourceType").toString() != QLatin1String("Observation")) {
    return std::unexpected(QStringLiteral("resourceType is not \"Observation\""));
  }

  const QString deviceId = root->value("deviceId").toString();
  if (deviceId.isEmpty()) {
    return std::unexpected(QStringLiteral("missing required field: deviceId"));
  }

  // `status` must be present; its value is not constrained at the parsing boundary.
  if (!root->value("status").isString()) {
    return std::unexpected(QStringLiteral("missing required field: status"));
  }

  const auto effectiveTime = json_message::parseUtcTime(root.value(), QStringLiteral("effectiveDateTime"));
  if (!effectiveTime.has_value()) {
    return std::unexpected(effectiveTime.error());
  }

  const QJsonValue codeValue = root->value("code");
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

  const QJsonValue quantityValue = root->value("valueQuantity");
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
      .effectiveTime = effectiveTime.value(),
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
