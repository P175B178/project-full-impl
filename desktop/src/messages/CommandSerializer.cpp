#include "messages/CommandSerializer.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace warden::messages::command_serializer {

namespace {

QByteArray serialize(const QJsonObject &object) { return QJsonDocument(object).toJson(QJsonDocument::Compact); }

} // namespace

QByteArray temperatureThreshold(double celsius) {
  return serialize({{"value", celsius}, {"unit", QString::fromUtf8("°C")}});
}

QByteArray humidityThreshold(double percent) { return serialize({{"value", percent}, {"unit", "%"}}); }

QByteArray interval(int seconds) { return serialize({{"seconds", seconds}}); }

QByteArray forcedState(status_message::State state) {
  return serialize({{"state", status_message::stateName(state).toLower()}});
}

} // namespace warden::messages::command_serializer
