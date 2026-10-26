#include "app/MainWindow.h"
#include "messages/DeviceGateway.h"
#include "mqtt/MqttConnection.h"
#include "sim/SimulatedGateway.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include <memory>
#include <utility>

int main(int argc, char *argv[]) {
  QApplication app{argc, argv};
  // QSettings stores the settings under these names.
  QApplication::setOrganizationName("Warden");
  QApplication::setApplicationName("Warden Console");
  QApplication::setApplicationVersion(WARDEN_CONSOLE_VERSION);

  QCommandLineParser options;
  options.setApplicationDescription(QStringLiteral("Monitors Warden devices through an MQTT broker."));
  options.addHelpOption();
  options.addVersionOption();
  const QCommandLineOption simulate(QStringLiteral("simulate"),
                                    QStringLiteral("Show two simulated devices instead of a broker's."));
  options.addOption(simulate);
  options.process(app);

  // Only here is it decided where the devices come from: the window works with either source.
  std::unique_ptr<warden::messages::DeviceGateway> gateway;
  warden::mqtt::MqttConnection *broker = nullptr;
  if (options.isSet(simulate)) {
    gateway = std::make_unique<warden::sim::SimulatedGateway>();
  } else {
    auto connection = std::make_unique<warden::mqtt::MqttConnection>();
    broker          = connection.get();
    gateway         = std::move(connection);
  }

  warden::app::MainWindow window(*gateway, broker);
  window.show();

  return QApplication::exec();
}
