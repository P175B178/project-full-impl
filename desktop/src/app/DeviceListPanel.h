#pragma once

#include <QString>
#include <QWidget>

#include <memory>

class QSortFilterProxyModel;

namespace warden::core {
class DeviceModel;
} // namespace warden::core

namespace Ui {
class DeviceListPanel;
}

namespace warden::app {

/// The device list: a filter box over a sortable table (a `QSortFilterProxyModel`).
class DeviceListPanel : public QWidget {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DeviceListPanel)

public:
  explicit DeviceListPanel(QWidget *parent = nullptr);
  ~DeviceListPanel() override;

  void setModel(core::DeviceModel *devices);

private:
  std::unique_ptr<Ui::DeviceListPanel> m_ui;
  QSortFilterProxyModel *m_proxy;
};

} // namespace warden::app
