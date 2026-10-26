#pragma once

#include <QString>
#include <QtEnvironmentVariables>

namespace warden::mqtt {

/// Everything needed to open a broker connection.
struct BrokerSettings {
  static constexpr quint16 defaultPort = 1883;

  // The broker runs on the host: from the dev container, Docker's host.docker.internal, which
  // devcontainer.json sets here; elsewhere, this computer.
  QString host = qEnvironmentVariable("WARDEN_BROKER_HOST", QStringLiteral("localhost"));
  quint16 port = defaultPort;
  QString username; // empty: no login
  QString password;
  bool useTls = false;
  QString caCertPath; // a PEM CA trusted besides the system's
};

} // namespace warden::mqtt
