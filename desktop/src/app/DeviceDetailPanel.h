#pragma once

#include <QHash>
#include <QString>
#include <QWidget>

#include <memory>
#include <optional>

namespace warden::core {
struct DeviceState;
class HistoryModel;
} // namespace warden::core

namespace warden::messages {
class DeviceGateway;
} // namespace warden::messages

namespace Ui {
class DeviceDetailPanel;
}

namespace warden::app {

/// The selected device, laid out in DeviceDetailPanel.ui. Its controls send commands to it, and
/// show the settings it was last sent.
class DeviceDetailPanel : public QWidget {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DeviceDetailPanel)

public:
  explicit DeviceDetailPanel(QWidget *parent = nullptr);
  ~DeviceDetailPanel() override;

  void setGateway(messages::DeviceGateway *gateway);

  /// The controls are enabled while the device is online and the gateway connected. `history`, the
  /// device's, gets the thresholds the console knows, to highlight the readings above them.
  void showDevice(const core::DeviceState &device, core::HistoryModel *history);

  /// For when no device is selected.
  void clearDevice();

  /// Forgets every device's settings, for a new source of device data.
  void forgetDevices();

  /// The setting a device was last sent; nullopt when it was cleared.
  void setTemperatureThreshold(const QString &groupId, std::optional<double> celsius);
  void setHumidityThreshold(const QString &groupId, std::optional<double> percent);
  void setInterval(const QString &groupId, std::optional<int> seconds);

private:
  // nullopt until the console learns the setting.
  struct DeviceConfig {
    std::optional<double> temperatureThreshold;
    std::optional<double> humidityThreshold;
    std::optional<int> interval;
  };

  void loadEditors(const DeviceConfig &config);
  void showSettings(const DeviceConfig &config);
  void applyThresholds();
  void publishTemperatureThreshold();
  void publishHumidityThreshold();
  void publishInterval();
  void publishForcedState();

  std::unique_ptr<Ui::DeviceDetailPanel> m_ui;
  messages::DeviceGateway *m_gateway = nullptr;
  core::HistoryModel *m_history      = nullptr; // the shown device's
  QHash<QString, DeviceConfig> m_configs;
  QString m_currentGroup; // empty when no device is shown
};

} // namespace warden::app
