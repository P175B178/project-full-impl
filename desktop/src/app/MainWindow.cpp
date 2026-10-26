#include "app/MainWindow.h"

#include "app/StateColorDelegate.h"
#include "core/DeviceModel.h"
#include "messages/DeviceGateway.h"

#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QStatusBar>
#include <QTableView>

namespace warden::app {

namespace {

constexpr int defaultWidth  = 960;
constexpr int defaultHeight = 640;

QString stateText(messages::DeviceGateway::State state) {
  switch (state) {
  case messages::DeviceGateway::State::Disconnected:
    return QStringLiteral("Disconnected");
  case messages::DeviceGateway::State::Connecting:
    return QStringLiteral("Connecting…");
  case messages::DeviceGateway::State::Connected:
    return QStringLiteral("Connected");
  }
  return {};
}

} // namespace

MainWindow::MainWindow(messages::DeviceGateway &gateway, QWidget *parent)
      : QMainWindow(parent)
      , m_devices(new core::DeviceModel(this))
      , m_gateway(&gateway)
      , m_connectionState(new QLabel(this)) {
  setWindowTitle(QApplication::applicationName());
  resize(defaultWidth, defaultHeight);

  auto *deviceView = new QTableView(this);
  deviceView->setModel(m_devices);
  deviceView->setItemDelegateForColumn(core::DeviceModel::stateColumn, new StateColorDelegate(this));
  deviceView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // equal columns
  deviceView->verticalHeader()->setVisible(false);
  setCentralWidget(deviceView);

  // The gateway only emits; the window routes its signals into the model.
  connect(m_gateway, &messages::DeviceGateway::readingReceived, m_devices, &core::DeviceModel::applyReading);
  connect(m_gateway, &messages::DeviceGateway::statusReceived, m_devices, &core::DeviceModel::applyStatus);

  // The gateway's state stays in view, at the right of the status bar.
  statusBar()->addPermanentWidget(m_connectionState);
  connect(m_gateway, &messages::DeviceGateway::stateChanged, this,
          [this](messages::DeviceGateway::State state) { m_connectionState->setText(stateText(state)); });

  m_gateway->start();
}

} // namespace warden::app
