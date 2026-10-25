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

MosquittoClient::MosquittoClient() {
  // The library must be set up once per program, before its first use. It is never cleaned up: the
  // program ends with the client anyway.
  static std::once_flag libraryReady;
  std::call_once(libraryReady, [] { mosquitto_lib_init(); });

  // No client id, so the library makes one up; a clean session, so the broker keeps nothing for
  // this client between connections.
  m_handle.reset(mosquitto_new(nullptr, /*clean_session=*/true, this));
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
