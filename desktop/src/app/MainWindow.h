#pragma once

#include <QMainWindow>
#include <QString>

#include <memory>

class QLabel;

namespace warden::core {
class DeviceModel;
class DeviceMonitor;
} // namespace warden::core

namespace warden::messages {
class DeviceGateway;
} // namespace warden::messages

namespace Ui {
class MainWindow;
}

namespace warden::app {

/// Top-level window: it owns the device data and shows it. The data comes from the gateway that
/// main() hands it.
/// Laid out in MainWindow.ui; it wires the panels together.
class MainWindow : public QMainWindow {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MainWindow)

public:
  explicit MainWindow(messages::DeviceGateway &gateway, QWidget *parent = nullptr);
  ~MainWindow() override;

private:
  void showDevice(const QString &groupId);
  void updateScope(); // apply the ticked devices to the charts

  std::unique_ptr<Ui::MainWindow> m_ui;
  core::DeviceModel *m_devices;
  core::DeviceMonitor *m_monitor;
  messages::DeviceGateway *m_gateway; // main() owns it
  QLabel *m_connectionState;          // permanent, at the right of the status bar
  QString m_currentGroup;             // selected device, empty when none
};

} // namespace warden::app
