#include "messages/MessageRouter.h"

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

std::expected<Reading, QString> routeReading(const Topic &topic, const QByteArray &payload) {
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

} // namespace

std::expected<Reading, QString> route(const QString &topicName, const QByteArray &payload) {
  const QStringList segments = topicName.split(QLatin1Char('/'));
  if (segments.size() != topicSegments) {
    return unexpectedTopic(topicName);
  }
  const Topic topic{.name = topicName, .group = segments.at(1)};

  if (topic.name == topics::temperature(topic.group) || topic.name == topics::humidity(topic.group)) {
    return routeReading(topic, payload);
  }
  return unexpectedTopic(topicName);
}

} // namespace warden::messages::message_router
