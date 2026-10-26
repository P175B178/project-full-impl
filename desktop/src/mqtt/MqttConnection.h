#pragma once

#include "messages/DeviceGateway.h"
#include "mqtt/BrokerSettings.h"

#include <QByteArray>
#include <QList>
#include <QMqttClient>
#include <QSslError>
#include <QString>

#include <memory>
#include <optional>

class QTimer;

namespace warden::mqtt {

/// The `DeviceGateway` over a live `QMqttClient`: it subscribes to the topics of protocol/, runs
/// the payloads through the parsers in messages/ and emits the results.
class MqttConnection : public messages::DeviceGateway {
  Q_OBJECT

public:
  explicit MqttConnection(QObject *parent = nullptr);

  /// The broker the next start() connects to.
  void setBroker(const BrokerSettings &settings);

  /// Connect to the broker, dropping any current connection first. A failed or lost connection is
  /// retried every few seconds, unless the broker refused the login or its certificate failed the
  /// check.
  void start() override;
  void stop() override;
  [[nodiscard]] State state() const override;
  void publishTemperatureThreshold(const QString &groupId, double celsius) override;
  void publishHumidityThreshold(const QString &groupId, double percent) override;
  void publishInterval(const QString &groupId, int seconds) override;
  void publishForcedState(const QString &groupId, messages::status_message::State state) override;

signals:
  /// A human-readable description of a transport or authentication error, or of a command that
  /// could not be sent.
  void errorOccurred(const QString &message);

private:
  void createClient();
  void openConnection(const BrokerSettings &settings);
  void reportTlsErrors(const QList<QSslError> &errors);
  void subscribeAll();
  void publishConfig(const QString &topicName, const QByteArray &payload, bool retain);

  std::unique_ptr<QMqttClient> m_client;
  QTimer *m_reconnectTimer;
  BrokerSettings m_broker;
  /// The broker to stay connected to; empty when told to disconnect, or once retrying cannot help.
  std::optional<BrokerSettings> m_wanted;
};

} // namespace warden::mqtt
