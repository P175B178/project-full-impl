#pragma once

#include <QSet>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>

namespace warden::app {

/// Shows only the rows of the accepted devices, read from a data role: the alert log's rows of
/// the devices ticked in the list.
class GroupFilterProxyModel : public QSortFilterProxyModel {
  Q_OBJECT

public:
  explicit GroupFilterProxyModel(int groupRole, QObject *parent = nullptr);

  void setAcceptedGroups(const QStringList &groups);

protected:
  [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
  int m_groupRole;
  QSet<QString> m_accepted;
};

} // namespace warden::app
