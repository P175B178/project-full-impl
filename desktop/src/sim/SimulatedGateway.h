#pragma once

#include "messages/DeviceGateway.h"

#include <QByteArray>
#include <QString>

#include <array>

class QTimer;

namespace warden::sim {

/// A `DeviceGateway` that needs no broker: two made-up devices, each publishing on a timer of its
/// own. One device's temperature jumps now and then, the other stays steady. Their messages are
/// JSON on the device's topics, so the console parses them as it does a broker's. Like a device,
/// each takes the thresholds and the interval it is sent; like a broker, the simulator hands the
/// retained settings back.
class SimulatedGateway : public messages::DeviceGateway {
  Q_OBJECT

public:
  explicit SimulatedGateway(QObject *parent = nullptr);

  void start() override;
  void stop() override;
  [[nodiscard]] State state() const override;
  void publishTemperatureThreshold(const QString &groupId, double celsius) override;
  void publishHumidityThreshold(const QString &groupId, double percent) override;
  void publishInterval(const QString &groupId, int seconds) override;
  void publishForcedState(const QString &groupId, messages::status_message::State state) override;

private:
  /// One simulated device, with the settings it was last sent.
  struct Device {
    QString groupId;
    QString deviceId;
    double warmer; // °C above the others
    bool spikes;   // whether its temperature jumps now and then
    double temperatureThreshold;
    double humidityThreshold;
    int intervalSeconds;
    QTimer *timer;
    int counter;
  };

  Device *device(const QString &groupId); // nullptr for a group it does not simulate
  void tick(Device &device);
  void publishSettings(const Device &device);
  void publishConfig(const QString &topic, const QByteArray &payload, bool retain);

  std::array<Device, 2> m_devices; // fixed, so each timer can keep a reference to its device
  State m_state = State::Disconnected;
};

} // namespace warden::sim
