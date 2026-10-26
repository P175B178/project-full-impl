#include "app/MainWindow.h"
#include "sim/SimulatedGateway.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app{argc, argv};
  QApplication::setApplicationName("Warden Console");
  QApplication::setApplicationVersion(WARDEN_CONSOLE_VERSION);

  // The devices' messages are fixed by protocol/, so a simulator can stand in for the broker.
  warden::sim::SimulatedGateway gateway;
  warden::app::MainWindow window(gateway);
  window.show();

  return QApplication::exec();
}
