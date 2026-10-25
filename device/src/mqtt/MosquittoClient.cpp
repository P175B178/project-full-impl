#include "MosquittoClient.h"

#include <spdlog/spdlog.h>

#include <format>
#include <mutex>
#include <stdexcept>

namespace warden::mqtt {

namespace {

// How often the client and the broker exchange a message to check that the connection is still
// there, when nothing else is sent.
constexpr int keepAliveSeconds = 60;

} // namespace

MosquittoClient::MosquittoClient(const std::string &clientId) {
  // The library must be set up once per program, before its first use. It is never cleaned up: the
  // program ends with the client anyway.
  static std::once_flag libraryReady;
  std::call_once(libraryReady, [] { mosquitto_lib_init(); });

  // A clean session: the broker keeps nothing for this client between connections.
  m_handle.reset(mosquitto_new(clientId.c_str(), /*clean_session=*/true, this));
  if (!m_handle) {
    throw std::runtime_error{"Cannot create an MQTT client: out of memory"};
  }
  mosquitto_connect_callback_set(m_handle.get(), &MosquittoClient::onConnect);
  mosquitto_disconnect_callback_set(m_handle.get(), &MosquittoClient::onDisconnect);
}

MosquittoClient::~MosquittoClient() {
  if (m_connected) {
    mosquitto_disconnect(m_handle.get());
    mosquitto_loop_stop(m_handle.get(), /*force=*/false); // the thread ends after disconnecting
  }
}

void MosquittoClient::setWill(const MqttMessage &will) {
  if (const int result = mosquitto_will_set(m_handle.get(), will.topic.c_str(), static_cast<int>(will.payload.size()),
                                            will.payload.data(), will.qos, will.retained);
      result != MOSQ_ERR_SUCCESS) {
    throw std::runtime_error{std::format("Cannot set the MQTT Last Will: {}", mosquitto_strerror(result))};
  }
}

void MosquittoClient::connect(const MqttSettings &broker) {
  spdlog::info("Connecting to MQTT broker {}:{}", broker.host, broker.port);
  if (const int result = mosquitto_connect(m_handle.get(), broker.host.c_str(), broker.port, keepAliveSeconds);
      result != MOSQ_ERR_SUCCESS) {
    throw std::runtime_error{
        std::format("Cannot connect to MQTT broker {}:{}: {}", broker.host, broker.port, mosquitto_strerror(result))};
  }
  if (const int result = mosquitto_loop_start(m_handle.get()); result != MOSQ_ERR_SUCCESS) {
    throw std::runtime_error{std::format("Cannot start the MQTT network thread: {}", mosquitto_strerror(result))};
  }
  m_connected = true;
}

void MosquittoClient::publish(const MqttMessage &message) {
  if (const int result = mosquitto_publish(m_handle.get(), /*mid=*/nullptr, message.topic.c_str(),
                                           static_cast<int>(message.payload.size()), message.payload.data(),
                                           message.qos, message.retained);
      result != MOSQ_ERR_SUCCESS) {
    spdlog::error("Cannot publish to {}: {}", message.topic, mosquitto_strerror(result));
  }
}

void MosquittoClient::onConnect(mosquitto * /*handle*/, void * /*self*/, int result) {
  // The broker's answer to the connection request: 0 means accepted.
  if (result == 0) {
    spdlog::info("Connected to MQTT broker");
  } else {
    spdlog::error("MQTT broker refused the connection: {}", mosquitto_connack_string(result));
  }
}

void MosquittoClient::onDisconnect(mosquitto * /*handle*/, void * /*self*/, int reason) {
  // 0 means this program disconnected on purpose; anything else, the connection was lost.
  if (reason == 0) {
    spdlog::info("Disconnected from MQTT broker");
  } else {
    spdlog::warn("Lost the connection to the MQTT broker; reconnecting");
  }
}

} // namespace warden::mqtt
