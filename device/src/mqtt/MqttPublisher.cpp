#include "MqttPublisher.h"

#include "Payloads.h"

#include <format>
#include <utility>

namespace warden::mqtt {

MqttPublisher::MqttPublisher(MqttClient &client, std::string groupId, std::string deviceId)
      : m_client{client}
      , m_groupId{std::move(groupId)}
      , m_deviceId{std::move(deviceId)} {}

void MqttPublisher::reportReading(const SmoothedReading &reading, std::chrono::system_clock::time_point time) {
  m_client.publish({.topic   = std::format("sensors/{}/temperature", m_groupId),
                    .payload = temperatureObservation(m_deviceId, reading.temperature, time)});
  m_client.publish({.topic   = std::format("sensors/{}/humidity", m_groupId),
                    .payload = humidityObservation(m_deviceId, reading.humidity, time)});
}

} // namespace warden::mqtt
