#include "messages/ConfigMessage.h"
#include "messages/JsonMessage.h"

#include <QJsonObject>
#include <QJsonValue>

namespace warden::messages::config_message {

namespace {

std::expected<double, QString> parseThreshold(const QByteArray &payload, const QString &unit, double min, double max) {
  const auto root = json_message::parseObject(payload);
  if (!root.has_value()) {
    return std::unexpected(root.error());
  }
  const QJsonValue value = root->value("value");
  if (!value.isDouble()) {
    return std::unexpected(QStringLiteral("\"value\" must be a number"));
  }
  if (root->value("unit").toString() != unit) {
    return std::unexpected(QStringLiteral("\"unit\" must be \"%1\"").arg(unit));
  }
  const double threshold = value.toDouble();
  if (threshold < min || threshold > max) {
    return std::unexpected(QStringLiteral("threshold %1 is outside %2–%3").arg(threshold).arg(min).arg(max));
  }
  return threshold;
}

} // namespace

std::expected<double, QString> parseTemperatureThreshold(const QByteArray &payload) {
  return parseThreshold(payload, QString::fromUtf8("°C"), minTemperatureThreshold, maxTemperatureThreshold);
}

std::expected<double, QString> parseHumidityThreshold(const QByteArray &payload) {
  return parseThreshold(payload, QStringLiteral("%"), minHumidityThreshold, maxHumidityThreshold);
}

std::expected<int, QString> parseInterval(const QByteArray &payload) {
  const auto root = json_message::parseObject(payload);
  if (!root.has_value()) {
    return std::unexpected(root.error());
  }
  // toInteger() gives the fallback, outside the limits, for anything but a whole number.
  const qint64 seconds = root->value("seconds").toInteger(minInterval - 1);
  if (seconds < minInterval || seconds > maxInterval) {
    return std::unexpected(
        QStringLiteral("\"seconds\" must be a whole number in %1–%2").arg(minInterval).arg(maxInterval));
  }
  return static_cast<int>(seconds);
}

} // namespace warden::messages::config_message
