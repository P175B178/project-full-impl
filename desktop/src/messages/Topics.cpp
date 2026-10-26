#include "messages/Topics.h"

namespace warden::messages::topics {

QString temperature(const QString &groupId) { return QStringLiteral("sensors/%1/temperature").arg(groupId); }

QString humidity(const QString &groupId) { return QStringLiteral("sensors/%1/humidity").arg(groupId); }

QString status(const QString &groupId) { return QStringLiteral("sensors/%1/status").arg(groupId); }

QString temperatureThreshold(const QString &groupId) {
  return QStringLiteral("config/%1/temperature-threshold").arg(groupId);
}

QString humidityThreshold(const QString &groupId) {
  return QStringLiteral("config/%1/humidity-threshold").arg(groupId);
}

QString interval(const QString &groupId) { return QStringLiteral("config/%1/interval").arg(groupId); }

} // namespace warden::messages::topics
