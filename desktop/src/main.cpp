#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[]) {
  QApplication app{argc, argv};

  QMainWindow window;
  window.setWindowTitle("Hello from the desktop!");
  window.resize(960, 640);
  window.show();

  return QApplication::exec();
}
