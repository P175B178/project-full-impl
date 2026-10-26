#include "app/DeviceListPanel.h"
#include "ui_DeviceListPanel.h"

#include "app/StateColorDelegate.h"
#include "core/DeviceModel.h"

#include <QHeaderView>
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
}

DeviceListPanel::~DeviceListPanel() = default;

void DeviceListPanel::setModel(core::DeviceModel *devices) { m_proxy->setSourceModel(devices); }

} // namespace warden::app
