#include "app/DeviceDetailPanel.h"
#include "ui_DeviceDetailPanel.h"

#include "app/Colors.h"
#include "core/DeviceState.h"
#include "messages/Observation.h"
#include "messages/StatusMessage.h"

#include <QColor>
#include <QLabel>

#include <optional>

namespace warden::app {

namespace {

// A round indicator in the color of the device's state.
QString indicatorStyle(const core::DeviceState &device) {
  const QColor color = stateColor(device.hasStatus ? std::optional(device.state) : std::nullopt);
  return QStringLiteral("min-width:16px;max-width:16px;min-height:16px;max-height:16px;"
                        "border-radius:8px;background:%1;")
      .arg(color.name());
}

} // namespace

DeviceDetailPanel::DeviceDetailPanel(QWidget *parent)
      : QWidget(parent)
      , m_ui(std::make_unique<Ui::DeviceDetailPanel>()) {
  m_ui->setupUi(this);
  clearDevice();
}

DeviceDetailPanel::~DeviceDetailPanel() = default;

void DeviceDetailPanel::clearDevice() {
  setEnabled(false);
  m_ui->deviceNameLabel->setText(QStringLiteral("No device selected"));
  m_ui->stateLabel->setText(QStringLiteral("—"));
  m_ui->stateIndicator->setStyleSheet(QString());
  m_ui->temperatureValueLabel->setText(QStringLiteral("—"));
  m_ui->humidityValueLabel->setText(QStringLiteral("—"));
  m_ui->lastUpdateValueLabel->setText(QStringLiteral("—"));
}

void DeviceDetailPanel::showDevice(const core::DeviceState &device) {
  setEnabled(true);
  m_ui->deviceNameLabel->setText(device.displayName());
  m_ui->stateLabel->setText(device.hasStatus ? messages::status_message::stateName(device.state) : QStringLiteral("—"));
  m_ui->stateIndicator->setStyleSheet(indicatorStyle(device));
  m_ui->temperatureValueLabel->setText(messages::observation::format(device.temperature, device.temperatureUnit));
  m_ui->humidityValueLabel->setText(messages::observation::format(device.humidity, device.humidityUnit));
  // UTC, ending in Z, like every other time the console shows.
  m_ui->lastUpdateValueLabel->setText(device.lastUpdate.isValid() ? device.lastUpdate.toString(Qt::ISODate)
                                                                  : QStringLiteral("—"));
}

} // namespace warden::app
