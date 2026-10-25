#pragma once

#include "MqttClient.h"
#include "Reporter.h"
#include "State.h"
#include "StateMachine.h"

#include <chrono>
#include <string>

namespace warden::mqtt {

/// Turns what warden has to report into MQTT messages: the topics, payloads and delivery settings
/// of protocol/, for one group and device.
class MqttPublisher final : public Reporter {
public:
  MqttPublisher(MqttClient &client, std::string groupId, std::string deviceId);

  ~MqttPublisher() override                       = default;
  MqttPublisher(const MqttPublisher &)            = delete;
  MqttPublisher &operator=(const MqttPublisher &) = delete;
  MqttPublisher(MqttPublisher &&)                 = delete;
  MqttPublisher &operator=(MqttPublisher &&)      = delete;

  /// Publishes the averaged temperature and humidity, one message each, on
  /// sensors/<group-id>/temperature and sensors/<group-id>/humidity. Sent once and not retained: a
  /// newer reading follows soon anyway.
  void reportReading(const SmoothedReading &reading, std::chrono::system_clock::time_point time) override;

  /// Publishes the device's state on sensors/<group-id>/status. Retained, so whoever subscribes
  /// later learns the current state at once, and sent until the broker confirms it, since the next
  /// one may be a long time away.
  void reportState(State state, std::chrono::system_clock::time_point time) override;

  /// The status saying this device is offline: the Last Will, which the broker publishes if the
  /// device disappears without disconnecting.
  [[nodiscard]] MqttMessage offlineStatus(std::chrono::system_clock::time_point time) const;

private:
  [[nodiscard]] std::string statusTopic() const;

  MqttClient &m_client;
  std::string m_groupId;
  std::string m_deviceId;
};

} // namespace warden::mqtt
