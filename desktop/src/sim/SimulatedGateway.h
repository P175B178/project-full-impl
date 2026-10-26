#pragma once

#include "messages/DeviceGateway.h"

#include <QString>

#include <array>

class QTimer;

namespace warden::sim {

/// A `DeviceGateway` that needs no broker: two made-up devices, each publishing on a timer of its
/// own. One device's temperature jumps now and then, the other stays steady. Their messages are
/// JSON on the device's topics, so the console parses them as it will a broker's.
class SimulatedGateway : public messages::DeviceGateway {
  Q_OBJECT

public:
  explicit SimulatedGateway(QObject *parent = nullptr);

  void start() override;
  void stop() override;
  [[nodiscard]] State state() const override;

private:
  /// One simulated device.
  struct Device {
    QString groupId;
    QString deviceId;
    double warmer; // °C above the others
    bool spikes;   // whether its temperature jumps now and then
    QTimer *timer;
    int counter;
  };

  void tick(Device &device);

  std::array<Device, 2> m_devices; // fixed, so each timer can keep a reference to its device
  State m_state = State::Disconnected;
};

} // namespace warden::sim
