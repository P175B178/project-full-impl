#pragma once

#include <string>

namespace warden::mqtt {

/// One message to publish.
struct MqttMessage {
  std::string topic;
  std::string payload;
  /// Quality of service: 0 sends it once, and it may be lost; 1 repeats it until the broker
  /// confirms it.
  int qos{0};
  /// The broker keeps the last retained message of a topic and hands it to everyone who subscribes
  /// later.
  bool retained{false};
};

/// Abstract MQTT client interface: what warden needs from a connection to the broker.
class MqttClient {
public:
  MqttClient()                              = default;
  virtual ~MqttClient()                     = default;
  MqttClient(const MqttClient &)            = delete;
  MqttClient &operator=(const MqttClient &) = delete;
  MqttClient(MqttClient &&)                 = delete;
  MqttClient &operator=(MqttClient &&)      = delete;

  virtual void publish(const MqttMessage &message) = 0;
};

} // namespace warden::mqtt
