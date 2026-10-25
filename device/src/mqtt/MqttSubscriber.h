#pragma once

#include "Commands.h"
#include "MqttClient.h"

#include <string>

namespace warden::mqtt {

/// Turns MQTT messages into what warden has to do: the command topics of protocol/, for one group.
/// The counterpart of MqttPublisher.
class MqttSubscriber {
public:
  MqttSubscriber(MqttClient &client, std::string groupId);

  ~MqttSubscriber()                                 = default;
  MqttSubscriber(const MqttSubscriber &)            = delete;
  MqttSubscriber &operator=(const MqttSubscriber &) = delete;
  MqttSubscriber(MqttSubscriber &&)                 = delete;
  MqttSubscriber &operator=(MqttSubscriber &&)      = delete;

  /// Subscribes to the configuration commands, on config/<group-id>/...: a valid command goes to
  /// its handler, one that cannot be parsed is logged and dropped. An empty message clears a
  /// retained command on the broker: warden keeps the value it was sent until it restarts.
  void subscribeToCommands(const CommandHandlers &handlers);

private:
  MqttClient &m_client;
  std::string m_groupId;
};

} // namespace warden::mqtt
