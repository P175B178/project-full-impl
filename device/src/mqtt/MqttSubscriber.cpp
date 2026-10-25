#include "MqttSubscriber.h"

#include <spdlog/spdlog.h>

#include <format>
#include <utility>

namespace warden::mqtt {

namespace {

/// Subscribes to `topic`, parsing each message with `parse` and handing the value to `handler`, or
/// logging why it was dropped.
template <typename Parse, typename Handler>
void subscribeTo(MqttClient &client, const std::string &topic, const Parse &parse, const Handler &handler) {
  client.subscribe(topic, [topic, parse, handler](const std::string &payload) {
    if (payload.empty()) {
      spdlog::info("The command on {} was cleared on the broker; the current value stays until warden restarts", topic);
      return;
    }
    const auto value = parse(payload);
    if (!value) {
      spdlog::warn("Ignored the message on {}: {}", topic, value.error());
      return;
    }
    handler(*value);
  });
}

} // namespace

MqttSubscriber::MqttSubscriber(MqttClient &client, std::string groupId)
      : m_client{client}
      , m_groupId{std::move(groupId)} {}

void MqttSubscriber::subscribeToCommands(const CommandHandlers &handlers) {
  subscribeTo(m_client, std::format("config/{}/temperature-threshold", m_groupId), parseTemperatureThreshold,
              handlers.temperatureThreshold);
  subscribeTo(m_client, std::format("config/{}/humidity-threshold", m_groupId), parseHumidityThreshold,
              handlers.humidityThreshold);
  subscribeTo(m_client, std::format("config/{}/interval", m_groupId), parseInterval, handlers.interval);
  subscribeTo(m_client, std::format("config/{}/state", m_groupId), parseForcedState, handlers.state);
}

} // namespace warden::mqtt
