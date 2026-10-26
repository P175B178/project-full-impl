#include "messages/Topics.h"

namespace warden::messages::topics {

QString temperature(const QString &groupId) { return QStringLiteral("sensors/%1/temperature").arg(groupId); }

QString humidity(const QString &groupId) { return QStringLiteral("sensors/%1/humidity").arg(groupId); }

} // namespace warden::messages::topics
