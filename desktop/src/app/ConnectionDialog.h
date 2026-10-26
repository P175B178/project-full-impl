#pragma once

#include "mqtt/BrokerSettings.h"

#include <QDialog>

#include <memory>

namespace Ui {
class ConnectionDialog;
}

namespace warden::app {

/// Edits the broker connection, laid out in ConnectionDialog.ui: host and port. `settings()` gives
/// the result once accepted.
class ConnectionDialog : public QDialog {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ConnectionDialog)

public:
  explicit ConnectionDialog(const mqtt::BrokerSettings &initial, QWidget *parent = nullptr);
  ~ConnectionDialog() override;

  [[nodiscard]] mqtt::BrokerSettings settings() const;

private:
  std::unique_ptr<Ui::ConnectionDialog> m_ui;
};

} // namespace warden::app
