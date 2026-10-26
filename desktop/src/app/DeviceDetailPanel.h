#pragma once

#include <QWidget>

#include <memory>

namespace warden::core {
struct DeviceState;
} // namespace warden::core

namespace Ui {
class DeviceDetailPanel;
}

namespace warden::app {

/// The panel for the selected device, laid out in DeviceDetailPanel.ui.
class DeviceDetailPanel : public QWidget {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DeviceDetailPanel)

public:
  explicit DeviceDetailPanel(QWidget *parent = nullptr);
  ~DeviceDetailPanel() override;

  /// Show one device's current state.
  void showDevice(const core::DeviceState &device);

  /// For when no device is selected.
  void clearDevice();

private:
  std::unique_ptr<Ui::DeviceDetailPanel> m_ui;
};

} // namespace warden::app
