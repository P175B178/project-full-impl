#include "app/DeviceListPanel.h"
#include "ui_DeviceListPanel.h"

#include "app/StateColorDelegate.h"
#include "core/DeviceModel.h"
#include "core/DeviceState.h"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTableView>

namespace warden::app {

DeviceListPanel::DeviceListPanel(QWidget *parent)
      : QWidget(parent)
      , m_ui(std::make_unique<Ui::DeviceListPanel>())
      , m_proxy(new QSortFilterProxyModel(this)) {
  m_ui->setupUi(this);

  m_proxy->setFilterKeyColumn(core::DeviceModel::deviceColumn);
  m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_proxy->setSortRole(core::DeviceModel::sortRole);
  m_ui->deviceView->setModel(m_proxy);
  m_ui->deviceView->setItemDelegateForColumn(core::DeviceModel::stateColumn, new StateColorDelegate(this));
  m_ui->deviceView->setSortingEnabled(true);
  m_ui->deviceView->sortByColumn(core::DeviceModel::deviceColumn, Qt::AscendingOrder);
  m_ui->deviceView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // equal columns
  m_ui->deviceView->verticalHeader()->setVisible(false);

  connect(m_ui->deviceFilter, &QLineEdit::textChanged, m_proxy, &QSortFilterProxyModel::setFilterFixedString);
  // The selection, not the current row: Ctrl+click deselects a row but leaves it current.
  connect(m_ui->deviceView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] {
    const QModelIndexList rows = m_ui->deviceView->selectionModel()->selectedRows();
    const QModelIndex source   = rows.isEmpty() ? QModelIndex() : m_proxy->mapToSource(rows.first());
    if (m_devices == nullptr || !source.isValid()) {
      emit deviceSelected(QString());
      return;
    }
    emit deviceSelected(m_devices->deviceAt(source.row()).groupId);
  });
}

DeviceListPanel::~DeviceListPanel() = default;

void DeviceListPanel::setModel(core::DeviceModel *devices) {
  m_devices = devices;
  m_proxy->setSourceModel(devices);
}

} // namespace warden::app
