#include "app/ConnectionDialog.h"
#include "ui_ConnectionDialog.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QSpinBox>

namespace warden::app {

ConnectionDialog::ConnectionDialog(const mqtt::BrokerSettings &initial, QWidget *parent)
      : QDialog(parent)
      , m_ui(std::make_unique<Ui::ConnectionDialog>()) {
  m_ui->setupUi(this);
  m_ui->hostEdit->setText(initial.host);
  m_ui->portSpin->setValue(initial.port);

  connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ConnectionDialog::~ConnectionDialog() = default;

mqtt::BrokerSettings ConnectionDialog::settings() const {
  return mqtt::BrokerSettings{
      .host = m_ui->hostEdit->text().trimmed(),
      .port = static_cast<quint16>(m_ui->portSpin->value()),
  };
}

} // namespace warden::app
