#include "app/MainWindow.h"
#include "ui_MainWindow.h"

#include "app/ChartPanel.h"
#include "app/DeviceDetailPanel.h"
#include "app/DeviceListPanel.h"
#include "app/LogPanel.h"
#include "app/ViolationHighlightDelegate.h"
#include "core/DeviceModel.h"
#include "core/DeviceMonitor.h"
#include "core/DeviceState.h"
#include "core/HistoryModel.h"
#include "messages/DeviceGateway.h"

#include <QAbstractItemModel>
#include <QDockWidget>
#include <QHeaderView>
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
      , m_monitor(new core::DeviceMonitor(this))
      , m_gateway(&gateway)
      , m_connectionState(new QLabel(this)) {
  m_ui->setupUi(this);
  m_ui->historyView->setItemDelegate(new ViolationHighlightDelegate(m_ui->historyView));
  m_ui->historyView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // equal columns
  m_ui->historyView->verticalHeader()->setVisible(false);
  m_ui->deviceList->setModel(m_devices);
  resizeDocks({m_ui->deviceDock}, {deviceListWidth}, Qt::Horizontal); // room for every column
  resizeDocks({m_ui->logDock}, {logHeight}, Qt::Vertical);

  // The list selection drives the detail panel and the history, which refresh when the device's
  // data updates.
  connect(m_ui->deviceList, &DeviceListPanel::deviceSelected, this, &MainWindow::showDevice);
  connect(m_devices, &QAbstractItemModel::dataChanged, this, [this] { showDevice(m_currentGroup); });

  // View menu: show or hide the docks.
  auto *viewMenu = menuBar()->addMenu(QStringLiteral("&View"));
  viewMenu->addAction(m_ui->deviceDock->toggleViewAction());
  viewMenu->addAction(m_ui->logDock->toggleViewAction());

  // The gateway only emits; the window routes its signals into the models and the log.
  connect(m_gateway, &messages::DeviceGateway::readingReceived, m_devices, &core::DeviceModel::applyReading);
  connect(m_gateway, &messages::DeviceGateway::readingReceived, m_monitor, &core::DeviceMonitor::addReading);
  connect(m_gateway, &messages::DeviceGateway::readingReceived, m_ui->charts, &ChartPanel::addReading);
  connect(m_gateway, &messages::DeviceGateway::statusReceived, m_devices, &core::DeviceModel::applyStatus);
  connect(m_gateway, &messages::DeviceGateway::temperatureThresholdReceived, m_ui->detailPanel,
          &DeviceDetailPanel::setTemperatureThreshold);
  connect(m_gateway, &messages::DeviceGateway::humidityThresholdReceived, m_ui->detailPanel,
          &DeviceDetailPanel::setHumidityThreshold);
  connect(m_gateway, &messages::DeviceGateway::intervalReceived, m_ui->detailPanel, &DeviceDetailPanel::setInterval);
  connect(m_gateway, &messages::DeviceGateway::messageDropped, m_ui->logPanel, &LogPanel::appendMessage);
  connect(m_gateway, &messages::DeviceGateway::commandPublished, m_ui->logPanel, &LogPanel::appendMessage);

  // The gateway's state stays in view, at the right of the status bar.
  statusBar()->addPermanentWidget(m_connectionState);
  connect(m_gateway, &messages::DeviceGateway::stateChanged, this, [this](messages::DeviceGateway::State state) {
    m_connectionState->setText(stateText(state));
    m_ui->logPanel->appendMessage(stateText(state));
    showDevice(m_currentGroup); // the controls work only while connected
  });

  m_ui->detailPanel->setGateway(m_gateway);

  m_gateway->start();
}

MainWindow::~MainWindow() = default;

void MainWindow::showDevice(const QString &groupId) {
  m_currentGroup                  = groupId;
  const core::DeviceState *device = groupId.isEmpty() ? nullptr : m_devices->device(groupId);
  if (device == nullptr) {
    m_ui->detailPanel->clearDevice();
    m_ui->historyView->setModel(nullptr);
    m_ui->deviceStack->setCurrentWidget(m_ui->deviceHint);
    m_ui->historyStack->setCurrentWidget(m_ui->historyHint);
    return;
  }
  m_ui->deviceStack->setCurrentWidget(m_ui->detailPanel);
  m_ui->historyStack->setCurrentWidget(m_ui->historyPage);
  core::HistoryModel *history = m_monitor->historyFor(groupId);
  m_ui->detailPanel->showDevice(*device, history);
  if (m_ui->historyView->model() != history) {
    m_ui->historyView->setModel(history);
  }
}

} // namespace warden::app
