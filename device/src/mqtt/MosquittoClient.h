#pragma once

#include "Config.h"

#include <mosquitto.h>

#include <memory>
#include <string>

namespace warden::mqtt {

/// A connection to an MQTT broker through libmosquitto.
///
/// connect() opens the connection and starts its network thread, which also reconnects when the
/// connection is lost.
class MosquittoClient {
public:
  /// `clientId` names this client on the broker; it must be unique there.
  explicit MosquittoClient(const std::string &clientId);
  ~MosquittoClient();

  MosquittoClient(const MosquittoClient &)            = delete;
  MosquittoClient &operator=(const MosquittoClient &) = delete;
  MosquittoClient(MosquittoClient &&)                 = delete;
  MosquittoClient &operator=(MosquittoClient &&)      = delete;

  /// Connects to the broker. Throws std::runtime_error if it cannot be reached.
  void connect(const MqttSettings &broker);

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
