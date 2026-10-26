#include "app/DeviceDetailPanel.h"
#include "ui_DeviceDetailPanel.h"

#include "app/Colors.h"
#include "core/DeviceState.h"
#include "messages/DeviceGateway.h"
#include "messages/Observation.h"
#include "messages/StatusMessage.h"

#include <QColor>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>

#include <optional>

namespace warden::app {

namespace {

// The device's limits (device/config/warden.toml): it rejects a value outside them. The
// temperature limits are its default plausible range.
constexpr double minTemperatureThreshold = 0.0;
constexpr double maxTemperatureThreshold = 50.0;
constexpr double minHumidityThreshold    = 0.0;
constexpr double maxHumidityThreshold    = 100.0;
constexpr int minInterval                = 2;
constexpr int maxInterval                = 3600;

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
  m_ui->temperatureThresholdSpin->setRange(minTemperatureThreshold, maxTemperatureThreshold);
  m_ui->humidityThresholdSpin->setRange(minHumidityThreshold, maxHumidityThreshold);
  m_ui->intervalSpin->setRange(minInterval, maxInterval);
  m_ui->temperatureThresholdSlider->setRange(static_cast<int>(minTemperatureThreshold),
                                             static_cast<int>(maxTemperatureThreshold));
  m_ui->humidityThresholdSlider->setRange(static_cast<int>(minHumidityThreshold),
                                          static_cast<int>(maxHumidityThreshold));

  // Only Set sends a value: editing alone sends nothing.
  connect(m_ui->temperatureThresholdSetButton, &QPushButton::clicked, this,
          &DeviceDetailPanel::publishTemperatureThreshold);
  connect(m_ui->humidityThresholdSetButton, &QPushButton::clicked, this, &DeviceDetailPanel::publishHumidityThreshold);
  connect(m_ui->intervalSetButton, &QPushButton::clicked, this, &DeviceDetailPanel::publishInterval);

  // Each slider follows its spin box, in whole steps.
  connect(m_ui->temperatureThresholdSpin, &QDoubleSpinBox::valueChanged, this, [this](double value) {
    const QSignalBlocker blocker(m_ui->temperatureThresholdSlider);
    m_ui->temperatureThresholdSlider->setValue(qRound(value));
  });
  connect(m_ui->temperatureThresholdSlider, &QSlider::valueChanged, this, [this](int value) {
    const QSignalBlocker blocker(m_ui->temperatureThresholdSpin);
    m_ui->temperatureThresholdSpin->setValue(value);
  });
  connect(m_ui->humidityThresholdSpin, &QDoubleSpinBox::valueChanged, this, [this](double value) {
    const QSignalBlocker blocker(m_ui->humidityThresholdSlider);
    m_ui->humidityThresholdSlider->setValue(qRound(value));
  });
  connect(m_ui->humidityThresholdSlider, &QSlider::valueChanged, this, [this](int value) {
    const QSignalBlocker blocker(m_ui->humidityThresholdSpin);
    m_ui->humidityThresholdSpin->setValue(value);
  });

  clearDevice();
}

DeviceDetailPanel::~DeviceDetailPanel() = default;

void DeviceDetailPanel::setGateway(messages::DeviceGateway *gateway) { m_gateway = gateway; }

void DeviceDetailPanel::clearDevice() {
  m_currentGroup.clear();
  setEnabled(false);
  m_ui->deviceNameLabel->setText(QStringLiteral("No device selected"));
  m_ui->stateLabel->setText(QStringLiteral("—"));
  m_ui->stateIndicator->setStyleSheet(QString());
  m_ui->temperatureValueLabel->setText(QStringLiteral("—"));
  m_ui->humidityValueLabel->setText(QStringLiteral("—"));
  m_ui->lastUpdateValueLabel->setText(QStringLiteral("—"));
}

void DeviceDetailPanel::showDevice(const core::DeviceState &device) {
  m_currentGroup = device.groupId;
  setEnabled(true);
  m_ui->deviceNameLabel->setText(device.displayName());
  m_ui->stateLabel->setText(device.hasStatus ? messages::status_message::stateName(device.state) : QStringLiteral("—"));
  m_ui->stateIndicator->setStyleSheet(indicatorStyle(device));
  m_ui->temperatureValueLabel->setText(messages::observation::format(device.temperature, device.temperatureUnit));
  m_ui->humidityValueLabel->setText(messages::observation::format(device.humidity, device.humidityUnit));
  // UTC, ending in Z, like every other time the console shows.
  m_ui->lastUpdateValueLabel->setText(device.lastUpdate.isValid() ? device.lastUpdate.toString(Qt::ISODate)
                                                                  : QStringLiteral("—"));
  // Usable only while a command can reach the device.
  const bool connected = m_gateway != nullptr && m_gateway->state() == messages::DeviceGateway::State::Connected;
  m_ui->controlsGroup->setEnabled(device.online() && connected);
}

void DeviceDetailPanel::publishTemperatureThreshold() {
  if (!m_currentGroup.isEmpty() && m_gateway != nullptr) {
    m_gateway->publishTemperatureThreshold(m_currentGroup, m_ui->temperatureThresholdSpin->value());
  }
}

void DeviceDetailPanel::publishHumidityThreshold() {
  if (!m_currentGroup.isEmpty() && m_gateway != nullptr) {
    m_gateway->publishHumidityThreshold(m_currentGroup, m_ui->humidityThresholdSpin->value());
  }
}

void DeviceDetailPanel::publishInterval() {
  if (!m_currentGroup.isEmpty() && m_gateway != nullptr) {
    m_gateway->publishInterval(m_currentGroup, m_ui->intervalSpin->value());
  }
}

} // namespace warden::app
