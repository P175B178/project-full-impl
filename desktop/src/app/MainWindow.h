#pragma once

#include <QMainWindow>

class QLabel;

namespace warden::core {
class DeviceModel;
} // namespace warden::core

namespace warden::messages {
class DeviceGateway;
} // namespace warden::messages

namespace warden::app {

/// Top-level window: it owns the device data and shows it. The data comes from the gateway that
/// main() hands it.
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(messages::DeviceGateway &gateway, QWidget *parent = nullptr);

private:
  core::DeviceModel *m_devices;
  messages::DeviceGateway *m_gateway; // main() owns it
  QLabel *m_connectionState;      // permanent, at the right of the status bar
};

} // namespace warden::app
