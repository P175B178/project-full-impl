#include "app/DeviceDetailPanel.h"
#include "ui_DeviceDetailPanel.h"

#include "app/Colors.h"
#include "core/DeviceState.h"
#include "core/HistoryModel.h"
#include "messages/ConfigMessage.h"
#include "messages/DeviceGateway.h"
#include "messages/Observation.h"
#include "messages/StatusMessage.h"

#include <QColor>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QVariant>

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
  // The device rejects a value outside its limits, so the controls do not offer one.
  m_ui->temperatureThresholdSpin->setRange(messages::config_message::minTemperatureThreshold,
                                           messages::config_message::maxTemperatureThreshold);
  m_ui->humidityThresholdSpin->setRange(messages::config_message::minHumidityThreshold,
                                        messages::config_message::maxHumidityThreshold);
  m_ui->intervalSpin->setRange(messages::config_message::minInterval, messages::config_message::maxInterval);
  m_ui->temperatureThresholdSlider->setRange(static_cast<int>(messages::config_message::minTemperatureThreshold),
                                             static_cast<int>(messages::config_message::maxTemperatureThreshold));
  m_ui->humidityThresholdSlider->setRange(static_cast<int>(messages::config_message::minHumidityThreshold),
                                          static_cast<int>(messages::config_message::maxHumidityThreshold));

  using State = messages::status_message::State;
  for (const State state : {State::Normal, State::Warning, State::Alert, State::Fault}) {
    m_ui->forcedStateCombo->addItem(messages::status_message::stateName(state), static_cast<int>(state));
  }

  // Only Set sends a value: editing alone sends nothing.
  connect(m_ui->temperatureThresholdSetButton, &QPushButton::clicked, this,
          &DeviceDetailPanel::publishTemperatureThreshold);
  connect(m_ui->humidityThresholdSetButton, &QPushButton::clicked, this, &DeviceDetailPanel::publishHumidityThreshold);
  connect(m_ui->intervalSetButton, &QPushButton::clicked, this, &DeviceDetailPanel::publishInterval);
  connect(m_ui->forcedStateSetButton, &QPushButton::clicked, this, &DeviceDetailPanel::publishForcedState);

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
  m_history = nullptr;
  showSettings(DeviceConfig{});
}

void DeviceDetailPanel::showDevice(const core::DeviceState &device, core::HistoryModel *history) {
  if (device.groupId != m_currentGroup) {
    m_currentGroup = device.groupId;
    loadEditors(m_configs.value(m_currentGroup));
  }
  setEnabled(true);
  m_ui->deviceNameLabel->setText(device.displayName());
  m_ui->stateLabel->setText(device.hasStatus ? messages::status_message::stateName(device.state) : QStringLiteral("—"));
  m_ui->stateIndicator->setStyleSheet(indicatorStyle(device));
  m_ui->temperatureValueLabel->setText(messages::observation::format(device.temperature, device.temperatureUnit));
  m_ui->humidityValueLabel->setText(messages::observation::format(device.humidity, device.humidityUnit));
  // UTC, ending in Z, like every other time the console shows.
  m_ui->lastUpdateValueLabel->setText(device.lastUpdate.isValid() ? device.lastUpdate.toString(Qt::ISODate)
                                                                  : QStringLiteral("—"));
  m_history = history;
  showSettings(m_configs.value(m_currentGroup));
  applyThresholds();
  // Usable only while a command can reach the device.
  const bool connected = m_gateway != nullptr && m_gateway->state() == messages::DeviceGateway::State::Connected;
  m_ui->controlsGroup->setEnabled(device.online() && connected);
  m_ui->forcedStateGroup->setEnabled(device.online() && connected);
}

void DeviceDetailPanel::setTemperatureThreshold(const QString &groupId, std::optional<double> celsius) {
  m_configs[groupId].temperatureThreshold = celsius;
  if (groupId == m_currentGroup) {
    showSettings(m_configs.value(groupId));
    applyThresholds();
  }
}

void DeviceDetailPanel::setHumidityThreshold(const QString &groupId, std::optional<double> percent) {
  m_configs[groupId].humidityThreshold = percent;
  if (groupId == m_currentGroup) {
    showSettings(m_configs.value(groupId));
    applyThresholds();
  }
}

void DeviceDetailPanel::setInterval(const QString &groupId, std::optional<int> seconds) {
  m_configs[groupId].interval = seconds;
  if (groupId == m_currentGroup) {
    showSettings(m_configs.value(groupId));
  }
}

// Only on selection, so a setting that arrives does not overwrite an edit.
void DeviceDetailPanel::loadEditors(const DeviceConfig &config) {
  if (config.temperatureThreshold) {
    m_ui->temperatureThresholdSpin->setValue(*config.temperatureThreshold);
  }
  if (config.humidityThreshold) {
    m_ui->humidityThresholdSpin->setValue(*config.humidityThreshold);
  }
  if (config.interval) {
    m_ui->intervalSpin->setValue(*config.interval);
  }
}

void DeviceDetailPanel::showSettings(const DeviceConfig &config) {
  const QString unknown = QStringLiteral("unknown");
  m_ui->temperatureThresholdDeviceLabel->setText(
      QStringLiteral("Device: %1")
          .arg(config.temperatureThreshold
                   ? messages::observation::format(config.temperatureThreshold, QStringLiteral("°C"))
                   : unknown));
  m_ui->humidityThresholdDeviceLabel->setText(
      QStringLiteral("Device: %1")
          .arg(config.humidityThreshold ? messages::observation::format(config.humidityThreshold, QStringLiteral("%"))
                                        : unknown));
  m_ui->intervalDeviceLabel->setText(
      QStringLiteral("Device: %1").arg(config.interval ? QStringLiteral("%1 s").arg(*config.interval) : unknown));
}

// Only a threshold the console knows is highlighted.
void DeviceDetailPanel::applyThresholds() {
  if (m_history != nullptr) {
    const DeviceConfig config = m_configs.value(m_currentGroup);
    m_history->setTemperatureThreshold(config.temperatureThreshold);
    m_history->setHumidityThreshold(config.humidityThreshold);
  }
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

void DeviceDetailPanel::publishForcedState() {
  const QVariant state = m_ui->forcedStateCombo->currentData();
  if (!m_currentGroup.isEmpty() && m_gateway != nullptr && state.isValid()) {
    m_gateway->publishForcedState(m_currentGroup, static_cast<messages::status_message::State>(state.toInt()));
  }
}

} // namespace warden::app
