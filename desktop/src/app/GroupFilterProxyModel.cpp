#include "app/GroupFilterProxyModel.h"

namespace warden::app {

GroupFilterProxyModel::GroupFilterProxyModel(int groupRole, QObject *parent)
      : QSortFilterProxyModel(parent)
      , m_groupRole(groupRole) {}

void GroupFilterProxyModel::setAcceptedGroups(const QStringList &groups) {
  m_accepted = QSet<QString>(groups.cbegin(), groups.cend());
  invalidateFilter();
}

bool GroupFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
  const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
  return m_accepted.contains(index.data(m_groupRole).toString());
}

} // namespace warden::app
