#include "messages/DeviceGateway.h"
#include "messages/Observation.h"
#include "sim/SimulatedGateway.h"

#include <QApplication>
#include <QDebug>
#include <QMainWindow>

int main(int argc, char *argv[]) {
  QApplication app{argc, argv};
  QApplication::setApplicationName("Warden Console");
  QApplication::setApplicationVersion(WARDEN_CONSOLE_VERSION);

  QMainWindow window;
  window.setWindowTitle(QApplication::applicationName());
  window.resize(960, 640);
  window.show();

  // The devices' messages are fixed by protocol/, so a simulator can stand in for the broker. Until
  // the window shows them, its readings go to the terminal.
  warden::sim::SimulatedGateway gateway;
  QObject::connect(&gateway, &warden::messages::DeviceGateway::readingReceived,
                   [](const QString &groupId, const warden::messages::observation::Reading &reading) {
                     qInfo().noquote() << groupId << reading.deviceId
                                       << warden::messages::observation::format(reading.value, reading.unit);
                   });
  gateway.start();

  return QApplication::exec();
}
