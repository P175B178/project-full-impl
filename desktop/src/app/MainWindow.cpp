#include "app/MainWindow.h"
#include "ui_MainWindow.h"

#include "app/ChartPanel.h"
#include "app/DeviceDetailPanel.h"
#include "app/DeviceListPanel.h"
#include "app/GroupFilterProxyModel.h"
#include "app/LogPanel.h"
#include "app/ViolationHighlightDelegate.h"
#include "core/AlertLogModel.h"
#include "core/CsvExport.h"
#include "core/DeviceModel.h"
#include "core/DeviceMonitor.h"
#include "core/DeviceState.h"
#include "core/HistoryModel.h"
#include "messages/DeviceGateway.h"
#include "messages/StatusMessage.h"
#include "mqtt/MqttConnection.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QDateTime>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMenuBar>
#include <QPixmap>
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

MainWindow::MainWindow(messages::DeviceGateway &gateway, mqtt::MqttConnection *broker, QWidget *parent)
      : QMainWindow(parent)
      , m_ui(std::make_unique<Ui::MainWindow>())
      , m_devices(new core::DeviceModel(this))
      , m_monitor(new core::DeviceMonitor(this))
      , m_alerts(new core::AlertLogModel(this))
      , m_alertFilter(new GroupFilterProxyModel(core::AlertLogModel::groupIdRole, this))
      , m_gateway(&gateway)
      , m_broker(broker)
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

  // The alert log is shown through a filter, scoped to the ticked devices.
  m_alertFilter->setSourceModel(m_alerts);
  m_ui->alertView->setModel(m_alertFilter);
  m_ui->alertView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // equal columns
  m_ui->alertView->verticalHeader()->setVisible(false);

  // Ticking devices in the list picks the ones the charts and the alert log show.
  connect(m_devices, &core::DeviceModel::checkedDevicesChanged, this, &MainWindow::updateScope);
  updateScope();

  // Only a broker has errors to report; the simulator has none.
  if (m_broker != nullptr) {
    connect(m_broker, &mqtt::MqttConnection::errorOccurred, this, [this](const QString &message) {
      statusBar()->showMessage(QStringLiteral("Error: %1").arg(message));
      m_ui->logPanel->appendMessage(QStringLiteral("Error: %1").arg(message));
    });
  }

  // View menu: show or hide the docks.
  auto *viewMenu = menuBar()->addMenu(QStringLiteral("&View"));
  viewMenu->addAction(m_ui->deviceDock->toggleViewAction());
  viewMenu->addAction(m_ui->logDock->toggleViewAction());

  auto *exportMenu = menuBar()->addMenu(QStringLiteral("&Export"));
  connect(exportMenu->addAction(QStringLiteral("History (&CSV)…")), &QAction::triggered, this,
          &MainWindow::exportHistoryCsv);
  connect(exportMenu->addAction(QStringLiteral("Charts (&PNG)…")), &QAction::triggered, this,
          &MainWindow::exportChartsPng);

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
  connect(m_gateway, &messages::DeviceGateway::statusReceived, m_alerts,
          [this](const QString &groupId, const messages::status_message::Status &status) {
            m_alerts->recordStatus(groupId, status, QDateTime::currentDateTimeUtc());
          });
  connect(m_gateway, &messages::DeviceGateway::messageDropped, m_ui->logPanel, &LogPanel::appendMessage);
  connect(m_gateway, &messages::DeviceGateway::commandPublished, m_ui->logPanel, &LogPanel::appendMessage);

  // The gateway's state stays in view, at the right of the status bar.
  statusBar()->addPermanentWidget(m_connectionState);
  connect(m_gateway, &messages::DeviceGateway::stateChanged, this, [this](messages::DeviceGateway::State state) {
    m_connectionState->setText(stateText(state));
    m_ui->logPanel->appendMessage(stateText(state));
    showDevice(m_currentGroup); // the controls work only while connected
    if (state == messages::DeviceGateway::State::Connected) {
      statusBar()->clearMessage(); // "Connecting to …" is over, and so is any earlier error
    }
  });

  m_ui->detailPanel->setGateway(m_gateway);

  if (m_broker == nullptr) {
    m_gateway->start();
    statusBar()->showMessage(QStringLiteral("Showing simulated devices, not a broker's"));
    m_ui->logPanel->appendMessage(QStringLiteral("Showing simulated devices"));
  } else {
    connectToBroker();
  }
}

MainWindow::~MainWindow() = default;

void MainWindow::exportHistoryCsv() {
  if (m_currentGroup.isEmpty()) {
    statusBar()->showMessage(QStringLiteral("Select a device to export its history."));
    return;
  }
  const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export History"),
                                                    QStringLiteral("history.csv"), QStringLiteral("CSV files (*.csv)"));
  if (path.isEmpty()) {
    return;
  }
  const QString error = core::csv_export::writeModel(m_monitor->historyFor(m_currentGroup), path);
  statusBar()->showMessage(error.isEmpty() ? QStringLiteral("History exported to %1").arg(path)
                                           : QStringLiteral("Export failed: %1").arg(error));
}

void MainWindow::exportChartsPng() {
  const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export Charts"), QStringLiteral("charts.png"),
                                                    QStringLiteral("PNG images (*.png)"));
  if (path.isEmpty()) {
    return;
  }
  const QPixmap pixmap = m_ui->charts->grab();
  statusBar()->showMessage(pixmap.save(path) ? QStringLiteral("Charts exported to %1").arg(path)
                                             : QStringLiteral("Could not save image to %1").arg(path));
}

void MainWindow::connectToBroker() {
  const QString message = QStringLiteral("Connecting to %1:%2…").arg(m_settings.host).arg(m_settings.port);
  statusBar()->showMessage(message);
  m_ui->logPanel->appendMessage(message);
  m_broker->setBroker(m_settings);
  m_broker->start();
}

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

void MainWindow::updateScope() {
  const QStringList groups = m_devices->checkedGroups();
  m_ui->charts->setVisibleDevices(groups);
  m_alertFilter->setAcceptedGroups(groups);
  m_ui->trendsStack->setCurrentWidget(groups.isEmpty() ? m_ui->trendsHint : m_ui->trendsPage);
  m_ui->alertsStack->setCurrentWidget(groups.isEmpty() ? m_ui->alertsHint : m_ui->alertsPage);
}

} // namespace warden::app
