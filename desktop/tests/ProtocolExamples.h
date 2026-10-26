#pragma once

#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <stdexcept>

namespace warden::test {

/// Reads protocol/examples/<name>: the messages both programs must agree on.
/// WARDEN_PROTOCOL_EXAMPLES, set in tests/CMakeLists.txt, is that folder.
inline QByteArray protocolExample(const QString &name) {
  QFile file{QStringLiteral(WARDEN_PROTOCOL_EXAMPLES "/") + name};
  if (!file.open(QIODevice::ReadOnly)) {
    throw std::runtime_error{"cannot open protocol example " + file.fileName().toStdString()};
  }
  return file.readAll();
}

/// The example as a JSON object, for tests that change one field of it.
inline QJsonObject protocolExampleObject(const QString &name) {
  return QJsonDocument::fromJson(protocolExample(name)).object();
}

} // namespace warden::test
