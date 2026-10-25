#pragma once

#include "MqttClient.h"
#include "StateMachine.h"

#include <chrono>
#include <string>

namespace warden::mqtt {

/// Turns what warden has to report into MQTT messages: the topics, payloads and delivery settings
/// of protocol/, for one group and device.
class MqttPublisher {
public:
  MqttPublisher(MqttClient &client, std::string groupId, std::string deviceId);

  ~MqttPublisher()                                = default;
  MqttPublisher(const MqttPublisher &)            = delete;
  MqttPublisher &operator=(const MqttPublisher &) = delete;
  MqttPublisher(MqttPublisher &&)                 = delete;
  MqttPublisher &operator=(MqttPublisher &&)      = delete;

  /// Publishes the averaged temperature and humidity, one message each, on
  /// sensors/<group-id>/temperature and sensors/<group-id>/humidity. Sent once and not retained: a
  /// newer reading follows soon anyway.
  void reportReading(const SmoothedReading &reading, std::chrono::system_clock::time_point time);

private:
  MqttClient &m_client;
  std::string m_groupId;
  std::string m_deviceId;
};

} // namespace warden::mqtt
