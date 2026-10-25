#include "MosquittoClient.h"

#include <spdlog/spdlog.h>

#include <format>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace warden::mqtt {

namespace {

// How often the client and the broker exchange a message to check that the connection is still
// there, when nothing else is sent.
constexpr int keepAliveSeconds                  = 60;
// The longest wait between two attempts to connect.
constexpr unsigned int maxReconnectDelaySeconds = 30;

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
  mosquitto_publish_callback_set(m_handle.get(), &MosquittoClient::onPublish);
}

MosquittoClient::~MosquittoClient() { disconnect(); }

void MosquittoClient::disconnect() {
  if (m_started) {
    mosquitto_disconnect(m_handle.get());
    mosquitto_loop_stop(m_handle.get(), /*force=*/false); // the thread ends after disconnecting
    m_started = false;
  }
}

void MosquittoClient::setConnectedHandler(std::function<void()> handler) { m_connectedHandler = std::move(handler); }

void MosquittoClient::setWill(const MqttMessage &will) {
  if (const int result = mosquitto_will_set(m_handle.get(), will.topic.c_str(), static_cast<int>(will.payload.size()),
                                            will.payload.data(), will.qos, will.retained);
      result != MOSQ_ERR_SUCCESS) {
    throw std::runtime_error{std::format("Cannot set the MQTT Last Will: {}", mosquitto_strerror(result))};
  }
}

void MosquittoClient::connect(const MqttSettings &broker) {
  m_brokerName = std::format("{}:{}", broker.host, broker.port);
  spdlog::info("Connecting to MQTT broker {}", m_brokerName);

  // After a failed attempt, wait 1 s before the next one, then twice as long after every further
  // failure, up to 30 s.
  mosquitto_reconnect_delay_set(m_handle.get(), 1, maxReconnectDelaySeconds, /*reconnect_exponential_backoff=*/true);

  // Only starts connecting; the network thread completes it, or keeps trying. Even when this fails
  // at once, for an unknown host name say, the thread tries again later: the name may resolve on
  // another network.
  if (const int result = mosquitto_connect_async(m_handle.get(), broker.host.c_str(), broker.port, keepAliveSeconds);
      result != MOSQ_ERR_SUCCESS) {
    spdlog::warn("Cannot reach MQTT broker {} ({}); trying again in the background", m_brokerName,
                 mosquitto_strerror(result));
    m_reportedNotFound = true;
  }
  if (const int result = mosquitto_loop_start(m_handle.get()); result != MOSQ_ERR_SUCCESS) {
    throw std::runtime_error{std::format("Cannot start the MQTT network thread: {}", mosquitto_strerror(result))};
  }
  m_started = true;
}

void MosquittoClient::publish(const MqttMessage &message) {
  if (!m_online) {
    spdlog::debug("Not connected to the MQTT broker: dropped a message to {}", message.topic);
    return;
  }
  if (const int result = mosquitto_publish(m_handle.get(), /*mid=*/nullptr, message.topic.c_str(),
                                           static_cast<int>(message.payload.size()), message.payload.data(),
                                           message.qos, message.retained);
      result != MOSQ_ERR_SUCCESS) {
    spdlog::error("Cannot publish to {}: {}", message.topic, mosquitto_strerror(result));
  }
}

void MosquittoClient::publishAndWait(const MqttMessage &message, std::chrono::milliseconds timeout) {
  if (!m_online) {
    return;
  }
  std::unique_lock lock{m_publishMutex};
  m_awaitedMessageSent = false;
  int messageId        = 0;
  if (const int result =
          mosquitto_publish(m_handle.get(), &messageId, message.topic.c_str(), static_cast<int>(message.payload.size()),
                            message.payload.data(), message.qos, message.retained);
      result != MOSQ_ERR_SUCCESS) {
    spdlog::error("Cannot publish to {}: {}", message.topic, mosquitto_strerror(result));
    return;
  }
  // The lock is held since before publishing, so onPublish() cannot report the message before it is
  // awaited.
  m_awaitedMessageId = messageId;
  if (!m_published.wait_for(lock, timeout, [this] { return m_awaitedMessageSent; })) {
    spdlog::warn("The broker did not confirm the message to {} within {} ms", message.topic, timeout.count());
  }
  m_awaitedMessageId.reset();
}

void MosquittoClient::onPublish(mosquitto * /*handle*/, void *self, int messageId) {
  // Called once a message is sent: for QoS 1, when the broker confirmed it.
  auto &client = *static_cast<MosquittoClient *>(self);
  {
    const std::scoped_lock lock{client.m_publishMutex};
    if (client.m_awaitedMessageId != messageId) {
      return;
    }
    client.m_awaitedMessageSent = true;
  }
  client.m_published.notify_one();
}

void MosquittoClient::onConnect(mosquitto * /*handle*/, void *self, int result) {
  auto &client = *static_cast<MosquittoClient *>(self);
  // The broker's answer to the connection request: 0 means accepted.
  if (result != 0) {
    spdlog::error("MQTT broker {} refused the connection: {}", client.m_brokerName, mosquitto_connack_string(result));
    return;
  }
  spdlog::info("Connected to MQTT broker {}", client.m_brokerName);
  client.m_online           = true;
  client.m_wasOnline        = true;
  client.m_reportedNotFound = false;
  if (client.m_connectedHandler) {
    client.m_connectedHandler();
  }
}

void MosquittoClient::onDisconnect(mosquitto * /*handle*/, void *self, int reason) {
  auto &client    = *static_cast<MosquittoClient *>(self);
  client.m_online = false;
  // 0 means this program disconnected on purpose; anything else, the connection was lost or could
  // not be made. Each is logged once, not on every retry.
  if (reason == 0) {
    spdlog::info("Disconnected from MQTT broker {}", client.m_brokerName);
  } else if (client.m_wasOnline) {
    spdlog::warn("Lost the connection to MQTT broker {}; reconnecting in the background", client.m_brokerName);
    client.m_wasOnline        = false;
    client.m_reportedNotFound = true; // the failed reconnects that follow need no message of their own
  } else if (!client.m_reportedNotFound) {
    spdlog::warn("Cannot reach MQTT broker {}; trying again in the background", client.m_brokerName);
    client.m_reportedNotFound = true;
  }
}

} // namespace warden::mqtt
