#include "messages/MessageRouter.h"

#include "messages/ConfigMessage.h"
#include "messages/Topics.h"

#include <QLatin1Char>
#include <QStringList>

namespace warden::messages::message_router {

namespace {

// Every topic has three levels, the group id in the middle (see Topics.h).
constexpr int topicSegments = 3;

struct Topic {
  QString name; // the whole topic, for messages
  QString group;
};

std::unexpected<QString> unexpectedTopic(const QString &topic) {
  return std::unexpected(QStringLiteral("Ignored message on unexpected topic %1").arg(topic));
}

std::expected<Message, QString> routeReading(const Topic &topic, const QByteArray &payload) {
  const auto reading = observation::parse(payload);
  if (!reading.has_value()) {
    return std::unexpected(QStringLiteral("Dropped reading on %1: %2").arg(topic.name, reading.error()));
  }
  const bool isTemperatureTopic = topic.name == topics::temperature(topic.group);
  if ((reading->metric == observation::Metric::Temperature) != isTemperatureTopic) {
    return std::unexpected(QStringLiteral("Dropped reading on %1: its code does not match the topic").arg(topic.name));
  }
  return Reading{.groupId = topic.group, .reading = reading.value()};
}

std::expected<Message, QString> routeStatus(const Topic &topic, const QByteArray &payload) {
  const auto status = status_message::parse(payload);
  if (!status.has_value()) {
    return std::unexpected(QStringLiteral("Dropped status on %1: %2").arg(topic.name, status.error()));
  }
  return Status{.groupId = topic.group, .status = status.value()};
}

/// One setting: `parse` reads its payload, and `Setting` is the message it becomes.
template <typename Setting, typename Parse>
std::expected<Message, QString> routeSetting(const Topic &topic, const QByteArray &payload, const Parse &parse) {
  // An empty retained message removes a setting from the broker: it is not a malformed one.
  if (payload.isEmpty()) {
    return Setting{topic.group, std::nullopt};
  }
  const auto value = parse(payload);
  if (!value.has_value()) {
    return std::unexpected(QStringLiteral("Dropped setting on %1: %2").arg(topic.name, value.error()));
  }
  return Setting{topic.group, value.value()};
}

} // namespace

std::expected<Message, QString> route(const QString &topicName, const QByteArray &payload) {
  const QStringList segments = topicName.split(QLatin1Char('/'));
  if (segments.size() != topicSegments) {
    return unexpectedTopic(topicName);
  }
  const Topic topic{.name = topicName, .group = segments.at(1)};

  if (topic.name == topics::temperature(topic.group) || topic.name == topics::humidity(topic.group)) {
    return routeReading(topic, payload);
  }
  if (topic.name == topics::status(topic.group)) {
    return routeStatus(topic, payload);
  }
  if (topic.name == topics::temperatureThreshold(topic.group)) {
    return routeSetting<TemperatureThreshold>(topic, payload, config_message::parseTemperatureThreshold);
  }
  if (topic.name == topics::humidityThreshold(topic.group)) {
    return routeSetting<HumidityThreshold>(topic, payload, config_message::parseHumidityThreshold);
  }
  if (topic.name == topics::interval(topic.group)) {
    return routeSetting<Interval>(topic, payload, config_message::parseInterval);
  }
  return unexpectedTopic(topicName);
}

} // namespace warden::messages::message_router
