#pragma once

#include "Config.h"
#include "MqttClient.h"

#include <mosquitto.h>

#include <memory>
#include <string>

namespace warden::mqtt {

/// A connection to an MQTT broker through libmosquitto.
///
/// connect() opens the connection and starts its network thread, which also reconnects when the
/// connection is lost.
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

  /// Connects to the broker. Throws std::runtime_error if it cannot be reached.
  void connect(const MqttSettings &broker);

  /// Hands the message to the network thread, which sends it. Logs an error if that is not
  /// possible.
  void publish(const MqttMessage &message) override;

private:
  // libmosquitto calls these on its network thread.
  static void onConnect(mosquitto *handle, void *self, int result);
  static void onDisconnect(mosquitto *handle, void *self, int reason);

  struct HandleDeleter {
    void operator()(mosquitto *handle) const noexcept { mosquitto_destroy(handle); }
  };
  std::unique_ptr<mosquitto, HandleDeleter> m_handle;
  bool m_connected{false}; ///< Whether connect() succeeded, so the destructor has to disconnect
};

} // namespace warden::mqtt
