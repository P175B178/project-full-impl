#include "app/MainWindow.h"
#include "ui_MainWindow.h"

#include "app/DeviceDetailPanel.h"
#include "app/DeviceListPanel.h"
#include "app/LogPanel.h"
#include "core/DeviceModel.h"
#include "core/DeviceState.h"
#include "messages/DeviceGateway.h"

#include <QAbstractItemModel>
#include <QDockWidget>
#include <QLabel>
#include <QMenuBar>
#include <QStackedWidget>
#include <QStatusBar>

namespace warden::app {

namespace {

constexpr int deviceListWidth = 520;
constexpr int logHeight       = 140;

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
  resizeDocks({m_ui->deviceDock}, {deviceListWidth}, Qt::Horizontal); // room for every column
  resizeDocks({m_ui->logDock}, {logHeight}, Qt::Vertical);

  // The list selection drives the detail panel; refresh it when the device's data updates.
  connect(m_ui->deviceList, &DeviceListPanel::deviceSelected, this, &MainWindow::showDevice);
  connect(m_devices, &QAbstractItemModel::dataChanged, this, [this] { showDevice(m_currentGroup); });

  // View menu: show or hide the docks.
  auto *viewMenu = menuBar()->addMenu(QStringLiteral("&View"));
  viewMenu->addAction(m_ui->deviceDock->toggleViewAction());
  viewMenu->addAction(m_ui->logDock->toggleViewAction());

  // The gateway only emits; the window routes its signals into the model and the log.
  connect(m_gateway, &messages::DeviceGateway::readingReceived, m_devices, &core::DeviceModel::applyReading);
  connect(m_gateway, &messages::DeviceGateway::statusReceived, m_devices, &core::DeviceModel::applyStatus);
  connect(m_gateway, &messages::DeviceGateway::messageDropped, m_ui->logPanel, &LogPanel::appendMessage);

  // The gateway's state stays in view, at the right of the status bar.
  statusBar()->addPermanentWidget(m_connectionState);
  connect(m_gateway, &messages::DeviceGateway::stateChanged, this, [this](messages::DeviceGateway::State state) {
    m_connectionState->setText(stateText(state));
    m_ui->logPanel->appendMessage(stateText(state));
  });

  m_gateway->start();
}

MainWindow::~MainWindow() = default;

void MainWindow::showDevice(const QString &groupId) {
  m_currentGroup                  = groupId;
  const core::DeviceState *device = groupId.isEmpty() ? nullptr : m_devices->device(groupId);
  if (device == nullptr) {
    m_ui->detailPanel->clearDevice();
    m_ui->deviceStack->setCurrentWidget(m_ui->deviceHint);
    return;
  }
  m_ui->deviceStack->setCurrentWidget(m_ui->detailPanel);
  m_ui->detailPanel->showDevice(*device);
}

} // namespace warden::app
