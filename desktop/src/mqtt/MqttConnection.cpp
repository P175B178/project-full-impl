#include "mqtt/MqttConnection.h"

#include "messages/CommandSerializer.h"
#include "messages/Topics.h"

#include <QMqttTopicFilter>
#include <QMqttTopicName>
#include <QSsl>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QTimer>

namespace warden::mqtt {

namespace {

// The device's QoS: 0 for readings, which are frequent; 1 for the retained status and Last Will.
constexpr quint8 sensorQos = 0;
constexpr quint8 statusQos = 1;

// Wait before retrying a failed or lost connection.
constexpr int reconnectDelayMs = 3000;

// Commands go QoS 1. Thresholds and the interval are retained: the device gets the latest one when
// it (re)connects. A forced state lasts only until the next valid reading, so it is not: a device
// connecting later must not act on an old one. The console subscribes to the retained ones: they
// are the settings the device was last sent.
constexpr quint8 configQos = 1;

QString errorText(QMqttClient::ClientError error) {
  if (error == QMqttClient::TransportInvalid) {
    return QStringLiteral("transport error — is the broker reachable?");
  }
  if (error == QMqttClient::BadUsernameOrPassword || error == QMqttClient::NotAuthorized) {
    return QStringLiteral("not authorized — check the username and password");
  }
  return QStringLiteral("connection error (code %1)").arg(static_cast<int>(error));
}

messages::DeviceGateway::State mapState(QMqttClient::ClientState state) {
  switch (state) {
  case QMqttClient::Disconnected:
    return messages::DeviceGateway::State::Disconnected;
  case QMqttClient::Connecting:
    return messages::DeviceGateway::State::Connecting;
  case QMqttClient::Connected:
    return messages::DeviceGateway::State::Connected;
  }
  return messages::DeviceGateway::State::Disconnected;
}

} // namespace

MqttConnection::MqttConnection(QObject *parent)
      : DeviceGateway(parent)
      , m_reconnectTimer(new QTimer(this)) {
  m_reconnectTimer->setSingleShot(true);
  m_reconnectTimer->setInterval(reconnectDelayMs);
  connect(m_reconnectTimer, &QTimer::timeout, this, [this] {
    if (m_wanted && m_client->state() == QMqttClient::Disconnected) {
      openConnection(*m_wanted);
    }
  });
  createClient();
}

void MqttConnection::setBroker(const BrokerSettings &settings) { m_broker = settings; }

void MqttConnection::start() {
  m_wanted = m_broker;
  m_reconnectTimer->stop();
  // A client takes a new host only while disconnected, so a new broker gets a new client.
  createClient();
  openConnection(m_broker);
}

void MqttConnection::stop() {
  m_wanted.reset();
  m_reconnectTimer->stop();
  if (m_client->state() != QMqttClient::Disconnected) {
    m_client->disconnectFromHost();
  }
}

void MqttConnection::openConnection(const BrokerSettings &settings) {
  m_client->setHostname(settings.host);
  m_client->setPort(settings.port);
  m_client->setUsername(settings.username);
  m_client->setPassword(settings.password);
  if (!settings.useTls) {
    m_client->connectToHost();
    return;
  }
  // The system's CAs, plus the one in caCertPath: the brokers in broker/ and the course broker
  // have a CA of their own.
  QSslConfiguration tls = QSslConfiguration::defaultConfiguration();
  if (!settings.caCertPath.isEmpty()) {
    const QList<QSslCertificate> caCerts =
        QSslCertificate::fromPath(settings.caCertPath, QSsl::Pem, QSslCertificate::PatternSyntax::FixedString);
    if (caCerts.isEmpty()) {
      emit errorOccurred(QStringLiteral("no CA certificate in %1").arg(settings.caCertPath));
      m_wanted.reset(); // retrying cannot help
      return;
    }
    QList<QSslCertificate> trusted = tls.caCertificates();
    trusted.append(caCerts);
    tls.setCaCertificates(trusted);
  }
  m_client->connectToHostEncrypted(tls);
  // The client reports a failed certificate check only as a transport error; its socket knows why.
  if (auto *socket = qobject_cast<QSslSocket *>(m_client->transport()); socket != nullptr) {
    connect(socket, &QSslSocket::sslErrors, this, &MqttConnection::reportTlsErrors, Qt::UniqueConnection);
  }
}

void MqttConnection::reportTlsErrors(const QList<QSslError> &errors) {
  if (errors.isEmpty()) {
    return;
  }
  m_wanted.reset(); // retrying cannot fix the certificate
  emit errorOccurred(QStringLiteral("the broker's certificate failed the check: %1").arg(errors.first().errorString()));
}

void MqttConnection::createClient() {
  // Deleting the old client also drops its connections.
  m_client = std::make_unique<QMqttClient>();
  // Subscriptions do not survive a new connection, so they are made on every connect.
  connect(m_client.get(), &QMqttClient::connected, this, &MqttConnection::subscribeAll);
  connect(m_client.get(), &QMqttClient::messageReceived, this,
          [this](const QByteArray &payload, const QMqttTopicName &topic) { route(topic.name(), payload); });
  connect(m_client.get(), &QMqttClient::stateChanged, this, [this](QMqttClient::ClientState state) {
    emit stateChanged(mapState(state));
    if (state == QMqttClient::Disconnected && m_wanted && !m_reconnectTimer->isActive()) {
      m_reconnectTimer->start();
    }
  });
  connect(m_client.get(), &QMqttClient::errorChanged, this, [this](QMqttClient::ClientError error) {
    if (error == QMqttClient::NoError) {
      return;
    }
    if (error == QMqttClient::BadUsernameOrPassword || error == QMqttClient::NotAuthorized) {
      m_wanted.reset(); // retrying cannot fix a refused login
    }
    // After a failed certificate check, reportTlsErrors() said why; "transport error" would not.
    if (error != QMqttClient::TransportInvalid || m_wanted) {
      emit errorOccurred(errorText(error));
    }
  });
}

messages::DeviceGateway::State MqttConnection::state() const { return mapState(m_client->state()); }

void MqttConnection::subscribeAll() {
  m_client->subscribe(QMqttTopicFilter(messages::topics::temperature(messages::topics::anyGroup)), sensorQos);
  m_client->subscribe(QMqttTopicFilter(messages::topics::humidity(messages::topics::anyGroup)), sensorQos);
  m_client->subscribe(QMqttTopicFilter(messages::topics::status(messages::topics::anyGroup)), statusQos);
  m_client->subscribe(QMqttTopicFilter(messages::topics::temperatureThreshold(messages::topics::anyGroup)), configQos);
  m_client->subscribe(QMqttTopicFilter(messages::topics::humidityThreshold(messages::topics::anyGroup)), configQos);
  m_client->subscribe(QMqttTopicFilter(messages::topics::interval(messages::topics::anyGroup)), configQos);
}

void MqttConnection::publishTemperatureThreshold(const QString &groupId, double celsius) {
  publishConfig(messages::topics::temperatureThreshold(groupId),
                messages::command_serializer::temperatureThreshold(celsius),
                /*retain=*/true);
}

void MqttConnection::publishHumidityThreshold(const QString &groupId, double percent) {
  publishConfig(messages::topics::humidityThreshold(groupId), messages::command_serializer::humidityThreshold(percent),
                /*retain=*/true);
}

void MqttConnection::publishInterval(const QString &groupId, int seconds) {
  publishConfig(messages::topics::interval(groupId), messages::command_serializer::interval(seconds), /*retain=*/true);
}

void MqttConnection::publishForcedState(const QString &groupId, messages::status_message::State state) {
  publishConfig(messages::topics::forcedState(groupId), messages::command_serializer::forcedState(state),
                /*retain=*/false);
}

void MqttConnection::publishConfig(const QString &topicName, const QByteArray &payload, bool retain) {
  const QMqttTopicName topic(topicName);
  if (m_client->state() != QMqttClient::Connected) {
    emit errorOccurred(QStringLiteral("not connected, %1 not sent").arg(topic.name()));
    return;
  }
  if (m_client->publish(topic, payload, configQos, retain) < 0) {
    emit errorOccurred(QStringLiteral("could not send %1").arg(topic.name()));
    return;
  }
  emit commandPublished(QStringLiteral("Sent %1 %2").arg(topic.name(), QString::fromUtf8(payload)));
}

} // namespace warden::mqtt
