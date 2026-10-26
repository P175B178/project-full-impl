#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[]) {
  QApplication app{argc, argv};
  QApplication::setApplicationName("Warden Console");
  QApplication::setApplicationVersion(WARDEN_CONSOLE_VERSION);

  QMainWindow window;
  window.setWindowTitle(QApplication::applicationName());
  window.resize(960, 640);
  window.show();

  return QApplication::exec();
}
