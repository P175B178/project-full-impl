#pragma once

#include <QMainWindow>

#include <memory>

class QLabel;

namespace warden::core {
class DeviceModel;
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
/// Laid out in MainWindow.ui.
class MainWindow : public QMainWindow {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MainWindow)

public:
  explicit MainWindow(messages::DeviceGateway &gateway, QWidget *parent = nullptr);
  ~MainWindow() override;

private:
  std::unique_ptr<Ui::MainWindow> m_ui;
  core::DeviceModel *m_devices;
  messages::DeviceGateway *m_gateway; // main() owns it
  QLabel *m_connectionState;          // permanent, at the right of the status bar
};

} // namespace warden::app
