#include "messages/DeviceGateway.h"

#include "messages/MessageRouter.h"

#include <QLoggingCategory>

namespace warden::messages {

namespace {

Q_LOGGING_CATEGORY(lcGateway, "warden.gateway")

} // namespace

DeviceGateway::DeviceGateway(QObject *parent)
      : QObject(parent) {}

DeviceGateway::~DeviceGateway() = default;

void DeviceGateway::route(const QString &topic, const QByteArray &payload) {
  const auto received = message_router::route(topic, payload);
  if (!received.has_value()) {
    qCWarning(lcGateway).noquote() << received.error(); // a message that could not be used
    return;
  }
  emit readingReceived(received->groupId, received->reading);
}

} // namespace warden::messages
