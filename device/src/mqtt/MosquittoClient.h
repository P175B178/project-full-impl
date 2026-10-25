#pragma once

#include "Config.h"
#include "MqttClient.h"

#include <mosquitto.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace warden::mqtt {

/// A connection to an MQTT broker through libmosquitto.
///
/// connect() starts its network thread, which connects in the background and keeps trying, up to
/// 30 s apart. Messages published while not connected are dropped.
class MosquittoClient final : public MqttClient {
public:
  /// `clientId` names this client on the broker; it must be unique there.
  explicit MosquittoClient(const std::string &clientId);
  ~MosquittoClient() override;

  MosquittoClient(const MosquittoClient &)            = delete;
  MosquittoClient &operator=(const MosquittoClient &) = delete;
  MosquittoClient(MosquittoClient &&)                 = delete;
  MosquittoClient &operator=(MosquittoClient &&)      = delete;

  /// Sets the Last Will: what the broker publishes for this client if the connection ends without a
  /// disconnect, after a crash or a lost network. Must come before connect(). Throws
  /// std::runtime_error if libmosquitto refuses it.
  void setWill(const MqttMessage &will);

  /// Starts connecting to the broker, in the background. Throws std::runtime_error only if the
  /// network thread cannot be started.
  void connect(const MqttSettings &broker);

  /// Hands the message to the network thread, which sends it. Drops it while not connected, rather
  /// than logging an error for every reading.
  void publish(const MqttMessage &message) override;

  void setConnectedHandler(std::function<void()> handler) override;

  /// Disconnects and stops the network thread: no handler is called after this returns. Call it
  /// before whatever the handlers use is destroyed; the destructor does it too.
  void disconnect();

private:
  // libmosquitto calls these on its network thread.
  static void onConnect(mosquitto *handle, void *self, int result);
  static void onDisconnect(mosquitto *handle, void *self, int reason);

  struct HandleDeleter {
    void operator()(mosquitto *handle) const noexcept { mosquitto_destroy(handle); }
  };
  std::unique_ptr<mosquitto, HandleDeleter> m_handle;
  std::string m_brokerName;                 ///< host:port, for the log
  std::function<void()> m_connectedHandler; ///< Set before the network thread starts
  bool m_started{false};                    ///< Whether the network thread runs, so disconnect() stops it
  std::atomic<bool> m_online{false};        ///< Whether connected now; set on the network thread
  // Only touched on the network thread:
  bool m_wasOnline{false};        ///< Whether connected before, so a disconnect is a loss
  bool m_reportedNotFound{false}; ///< Whether "cannot reach" was logged since the last connection
};

} // namespace warden::mqtt
