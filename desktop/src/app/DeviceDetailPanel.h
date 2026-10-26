#pragma once

#include <QString>
#include <QWidget>

#include <memory>

namespace warden::core {
struct DeviceState;
} // namespace warden::core

namespace warden::messages {
class DeviceGateway;
} // namespace warden::messages

namespace Ui {
class DeviceDetailPanel;
}

namespace warden::app {

/// The selected device, laid out in DeviceDetailPanel.ui. Its controls send commands to it.
class DeviceDetailPanel : public QWidget {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DeviceDetailPanel)

public:
  explicit DeviceDetailPanel(QWidget *parent = nullptr);
  ~DeviceDetailPanel() override;

  void setGateway(messages::DeviceGateway *gateway);

  /// The controls are enabled while the device is online and the gateway connected.
  void showDevice(const core::DeviceState &device);

  /// For when no device is selected.
  void clearDevice();

private:
  void publishTemperatureThreshold();
  void publishHumidityThreshold();
  void publishInterval();

  std::unique_ptr<Ui::DeviceDetailPanel> m_ui;
  messages::DeviceGateway *m_gateway = nullptr;
  QString m_currentGroup; // empty when no device is shown
};

} // namespace warden::app
