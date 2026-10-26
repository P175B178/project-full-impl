#include "messages/DeviceGateway.h"

#include "messages/MessageRouter.h"

#include <QLoggingCategory>

#include <variant>

namespace warden::messages {

namespace {

Q_LOGGING_CATEGORY(lcGateway, "warden.gateway")

/// Combines lambdas into one callable with an overload per message type, for std::visit().
template <typename... Handlers> struct Overloaded : Handlers... {
  using Handlers::operator()...;
};

} // namespace

DeviceGateway::DeviceGateway(QObject *parent)
      : QObject(parent) {}

DeviceGateway::~DeviceGateway() = default;

void DeviceGateway::route(const QString &topic, const QByteArray &payload) {
  const auto message = message_router::route(topic, payload);
  if (!message.has_value()) {
    qCWarning(lcGateway).noquote() << message.error(); // a message that could not be used
    return;
  }
  std::visit(
      Overloaded{
          [this](const message_router::Reading &received) { emit readingReceived(received.groupId, received.reading); },
          [this](const message_router::Status &received) { emit statusReceived(received.groupId, received.status); },
      },
      message.value());
}

} // namespace warden::messages
