#include "messages/StatusMessage.h"
#include "messages/JsonMessage.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

#include <optional>

namespace warden::messages::status_message {

namespace {

std::optional<State> stateFromString(const QString &state) {
  if (state == QLatin1String("normal")) {
    return State::Normal;
  }
  if (state == QLatin1String("warning")) {
    return State::Warning;
  }
  if (state == QLatin1String("alert")) {
    return State::Alert;
  }
  if (state == QLatin1String("fault")) {
    return State::Fault;
  }
  if (state == QLatin1String("offline")) {
    return State::Offline;
  }
  return std::nullopt;
}

} // namespace

std::expected<Status, QString> parse(const QByteArray &payload) {
  const auto root = json_message::parseObject(payload);
  if (!root.has_value()) {
    return std::unexpected(root.error());
  }

  const QString deviceId = root->value("deviceId").toString();
  if (deviceId.isEmpty()) {
    return std::unexpected(QStringLiteral("missing required field: deviceId"));
  }

  const std::optional<State> state = stateFromString(root->value("state").toString());
  if (!state.has_value()) {
    return std::unexpected(QStringLiteral("missing or unknown state: \"%1\"").arg(root->value("state").toString()));
  }

  const auto timestamp = json_message::parseUtcTime(root.value(), QStringLiteral("timestamp"));
  if (!timestamp.has_value()) {
    return std::unexpected(timestamp.error());
  }

  return Status{
      .deviceId  = deviceId,
      .state     = state.value(),
      .timestamp = timestamp.value(),
  };
}

QString stateName(State state) {
  switch (state) {
  case State::Normal:
    return QStringLiteral("Normal");
  case State::Warning:
    return QStringLiteral("Warning");
  case State::Alert:
    return QStringLiteral("Alert");
  case State::Fault:
    return QStringLiteral("Fault");
  case State::Offline:
    return QStringLiteral("Offline");
  }
  return {};
}

} // namespace warden::messages::status_message
