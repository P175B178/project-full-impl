#include "app/MainWindow.h"
#include "ui_MainWindow.h"

#include "app/DeviceListPanel.h"
#include "core/DeviceModel.h"
#include "messages/DeviceGateway.h"

#include <QLabel>
#include <QStatusBar>

namespace warden::app {

namespace {

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
      , m_ui(std::make_unique<Ui::MainWindow>())
      , m_devices(new core::DeviceModel(this))
      , m_gateway(&gateway)
      , m_connectionState(new QLabel(this)) {
  m_ui->setupUi(this);
  m_ui->deviceList->setModel(m_devices);

  // The gateway only emits; the window routes its signals into the model.
  connect(m_gateway, &messages::DeviceGateway::readingReceived, m_devices, &core::DeviceModel::applyReading);
  connect(m_gateway, &messages::DeviceGateway::statusReceived, m_devices, &core::DeviceModel::applyStatus);

  // The gateway's state stays in view, at the right of the status bar.
  statusBar()->addPermanentWidget(m_connectionState);
  connect(m_gateway, &messages::DeviceGateway::stateChanged, this,
          [this](messages::DeviceGateway::State state) { m_connectionState->setText(stateText(state)); });

  m_gateway->start();
}

MainWindow::~MainWindow() = default;

} // namespace warden::app
