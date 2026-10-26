#include "app/ConnectionDialog.h"
#include "ui_ConnectionDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

namespace warden::app {

ConnectionDialog::ConnectionDialog(const mqtt::BrokerSettings &initial, QWidget *parent)
      : QDialog(parent)
      , m_ui(std::make_unique<Ui::ConnectionDialog>()) {
  m_ui->setupUi(this);
  m_ui->hostEdit->setText(initial.host);
  m_ui->portSpin->setValue(initial.port);
  m_ui->usernameEdit->setText(initial.username);
  m_ui->passwordEdit->setText(initial.password);
  m_ui->useTlsCheck->setChecked(initial.useTls);
  m_ui->caCertEdit->setText(initial.caCertPath);

  connect(m_ui->browseCertButton, &QPushButton::clicked, this, [this] {
    const QString path =
        QFileDialog::getOpenFileName(this, QStringLiteral("Select CA certificate"), m_ui->caCertEdit->text(),
                                     QStringLiteral("Certificates (*.crt *.pem);;All files (*)"));
    if (!path.isEmpty()) {
      m_ui->caCertEdit->setText(path);
    }
  });
  connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ConnectionDialog::~ConnectionDialog() = default;

mqtt::BrokerSettings ConnectionDialog::settings() const {
  return mqtt::BrokerSettings{
      .host       = m_ui->hostEdit->text().trimmed(),
      .port       = static_cast<quint16>(m_ui->portSpin->value()),
      .username   = m_ui->usernameEdit->text(),
      .password   = m_ui->passwordEdit->text(),
      .useTls     = m_ui->useTlsCheck->isChecked(),
      .caCertPath = m_ui->caCertEdit->text().trimmed(),
  };
}

} // namespace warden::app
