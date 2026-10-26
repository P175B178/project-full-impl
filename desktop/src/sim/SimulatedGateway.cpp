#include "sim/SimulatedGateway.h"

#include "messages/CommandSerializer.h"
#include "messages/StatusMessage.h"
#include "messages/Topics.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include <array>
#include <utility>

namespace warden::sim {

namespace {

constexpr int msPerSecond     = 1000;
constexpr int spikeCycle      = 12; // every Nth reading, a spiking device jumps
constexpr int variationCycle  = 4;  // values step up by 1 for this many readings, then restart
constexpr double normalTemp   = 22.0;
constexpr double spikeTemp    = 38.0;
constexpr double warmerTemp   = 2.0; // demo-2 runs this much warmer than demo-1
constexpr double baseHumidity = 48.0;

// The device's defaults (device/config/warden.toml), but the shortest interval it allows: a demo
// should move.
constexpr double defaultTemperatureThreshold = 28.0;
constexpr double defaultHumidityThreshold    = 70.0;
constexpr int defaultIntervalSeconds         = 2;

QByteArray compact(const QJsonObject &object) { return QJsonDocument(object).toJson(QJsonDocument::Compact); }

// A reading as the device publishes it (protocol/examples/observation-temperature.json).
QByteArray temperatureObservation(const QString &deviceId, double celsius, const QDateTime &time) {
  return compact({
      {"resourceType", "Observation"},
      {"deviceId", deviceId},
      {"status", "final"},
      {"effectiveDateTime", time.toString(Qt::ISODate)},
      {"code", QJsonObject{{"system", "http://loinc.org"}, {"code", "8310-5"}, {"display", "Body temperature"}}},
      {"valueQuantity", QJsonObject{{"value", celsius},
                                    {"unit", QString::fromUtf8("°C")},
                                    {"system", "http://unitsofmeasure.org"},
                                    {"code", "Cel"}}},
  });
}

// A reading as the device publishes it (protocol/examples/observation-humidity.json).
QByteArray humidityObservation(const QString &deviceId, double percent, const QDateTime &time) {
  return compact({
      {"resourceType", "Observation"},
      {"deviceId", deviceId},
      {"status", "final"},
      {"effectiveDateTime", time.toString(Qt::ISODate)},
      {"code", QJsonObject{{"system", "http://loinc.org"}, {"code", "13834-7"}, {"display", "Humidity"}}},
      {"valueQuantity",
       QJsonObject{{"value", percent}, {"unit", "%"}, {"system", "http://unitsofmeasure.org"}, {"code", "%"}}},
  });
}

// A status as the device publishes it (protocol/examples/status-*.json).
QByteArray status(const QString &deviceId, messages::status_message::State state, const QDateTime &time) {
  return compact({
      {"deviceId", deviceId},
      {"state", messages::status_message::stateName(state).toLower()},
      {"timestamp", time.toString(Qt::ISODate)},
  });
}

} // namespace

SimulatedGateway::SimulatedGateway(QObject *parent)
      : DeviceGateway(parent)
      , m_devices{{
            Device{.groupId              = QStringLiteral("demo-1"),
                   .deviceId             = QStringLiteral("demo-1-sim"),
                   .warmer               = 0.0,
                   .spikes               = true,
                   .temperatureThreshold = defaultTemperatureThreshold,
                   .humidityThreshold    = defaultHumidityThreshold,
                   .intervalSeconds      = defaultIntervalSeconds,
                   .timer                = new QTimer(this),
                   .counter              = 0},
            Device{.groupId              = QStringLiteral("demo-2"),
                   .deviceId             = QStringLiteral("demo-2-sim"),
                   .warmer               = warmerTemp,
                   .spikes               = false,
                   .temperatureThreshold = defaultTemperatureThreshold,
                   .humidityThreshold    = defaultHumidityThreshold,
                   .intervalSeconds      = defaultIntervalSeconds,
                   .timer                = new QTimer(this),
                   .counter              = 0},
        }} {
  for (Device &device : m_devices) {
    device.timer->setInterval(device.intervalSeconds * msPerSecond);
    connect(device.timer, &QTimer::timeout, this, [this, &device] { tick(device); });
  }
}

void SimulatedGateway::start() {
  m_state = State::Connecting;
  emit stateChanged(m_state);
  m_state = State::Connected;
  emit stateChanged(m_state);
  for (Device &device : m_devices) {
    publishSettings(device);
    device.timer->start();
    tick(device); // the first reading at once
  }
}

void SimulatedGateway::stop() {
  for (const Device &device : std::as_const(m_devices)) {
    device.timer->stop();
  }
  m_state = State::Disconnected;
  emit stateChanged(m_state);
}

messages::DeviceGateway::State SimulatedGateway::state() const { return m_state; }

void SimulatedGateway::publishTemperatureThreshold(const QString &groupId, double celsius) {
  if (Device *target = device(groupId); target != nullptr) {
    target->temperatureThreshold = celsius;
    publishConfig(messages::topics::temperatureThreshold(groupId),
                  messages::command_serializer::temperatureThreshold(celsius), /*retain=*/true);
  }
}

void SimulatedGateway::publishHumidityThreshold(const QString &groupId, double percent) {
  if (Device *target = device(groupId); target != nullptr) {
    target->humidityThreshold = percent;
    publishConfig(messages::topics::humidityThreshold(groupId),
                  messages::command_serializer::humidityThreshold(percent),
                  /*retain=*/true);
  }
}

void SimulatedGateway::publishInterval(const QString &groupId, int seconds) {
  if (Device *target = device(groupId); target != nullptr) {
    target->intervalSeconds = seconds;
    target->timer->setInterval(seconds * msPerSecond);
    publishConfig(messages::topics::interval(groupId), messages::command_serializer::interval(seconds),
                  /*retain=*/true);
  }
}

SimulatedGateway::Device *SimulatedGateway::device(const QString &groupId) {
  for (Device &device : m_devices) {
    if (device.groupId == groupId) {
      return &device;
    }
  }
  return nullptr;
}

void SimulatedGateway::tick(Device &device) {
  ++device.counter;
  const QDateTime now      = QDateTime::currentDateTimeUtc();
  const double variation   = device.counter % variationCycle;
  const bool spike         = device.spikes && (device.counter % spikeCycle) == 0;
  const double temperature = spike ? spikeTemp : normalTemp + device.warmer + variation;
  const double humidity    = baseHumidity + variation;

  // As on the device: a value above its threshold is exceeded; one exceeded is Warning, both Alert.
  constexpr std::array byExceeded{messages::status_message::State::Normal, messages::status_message::State::Warning,
                                  messages::status_message::State::Alert};
  const auto exceeded = static_cast<std::size_t>(temperature > device.temperatureThreshold) +
                        static_cast<std::size_t>(humidity > device.humidityThreshold);

  route(messages::topics::temperature(device.groupId), temperatureObservation(device.deviceId, temperature, now));
  route(messages::topics::humidity(device.groupId), humidityObservation(device.deviceId, humidity, now));
  route(messages::topics::status(device.groupId), status(device.deviceId, byExceeded.at(exceeded), now));
}

// What a broker hands a new subscriber: the retained settings.
void SimulatedGateway::publishSettings(const Device &device) {
  route(messages::topics::temperatureThreshold(device.groupId),
        messages::command_serializer::temperatureThreshold(device.temperatureThreshold));
  route(messages::topics::humidityThreshold(device.groupId),
        messages::command_serializer::humidityThreshold(device.humidityThreshold));
  route(messages::topics::interval(device.groupId), messages::command_serializer::interval(device.intervalSeconds));
}

void SimulatedGateway::publishConfig(const QString &topic, const QByteArray &payload, bool retain) {
  emit commandPublished(QStringLiteral("Sent %1 %2").arg(topic, QString::fromUtf8(payload)));
  // A broker hands a retained message to every subscriber, this console among them.
  if (retain) {
    route(topic, payload);
  }
}

} // namespace warden::sim
