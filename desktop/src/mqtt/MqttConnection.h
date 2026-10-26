#pragma once

#include "messages/DeviceGateway.h"
#include "mqtt/BrokerSettings.h"

#include <QByteArray>
#include <QMqttClient>
#include <QString>

#include <memory>

namespace warden::mqtt {

/// The `DeviceGateway` over a live `QMqttClient`: it subscribes to the topics of protocol/, runs
/// the payloads through the parsers in messages/ and emits the results.
class MqttConnection : public messages::DeviceGateway {
  Q_OBJECT

public:
  explicit MqttConnection(QObject *parent = nullptr);

  /// The broker the next start() connects to.
  void setBroker(const BrokerSettings &settings);

  void start() override;
  void stop() override;
  [[nodiscard]] State state() const override;
  void publishTemperatureThreshold(const QString &groupId, double celsius) override;
  void publishHumidityThreshold(const QString &groupId, double percent) override;
  void publishInterval(const QString &groupId, int seconds) override;
  void publishForcedState(const QString &groupId, messages::status_message::State state) override;

signals:
  /// A human-readable description of a transport error, or of a command that could not be sent.
  void errorOccurred(const QString &message);

private:
  void createClient();
  void subscribeAll();
  void publishConfig(const QString &topicName, const QByteArray &payload, bool retain);

  std::unique_ptr<QMqttClient> m_client;
  BrokerSettings m_broker;
};

} // namespace warden::mqtt
