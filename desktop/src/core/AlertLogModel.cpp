#include "core/AlertLogModel.h"

#include <QString>

namespace warden::core {

AlertLogModel::AlertLogModel(QObject *parent)
      : QAbstractTableModel(parent) {}

int AlertLogModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
}

int AlertLogModel::columnCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : columnTotal; }

QVariant AlertLogModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
    return {};
  }
  const Entry &entry = m_entries.at(index.row());
  if (role == groupIdRole) {
    return entry.groupId;
  }
  if (role != Qt::DisplayRole) {
    return {};
  }
  switch (index.column()) {
  case timeColumn:
    return entry.time.toString(Qt::ISODate);
  case deviceColumn:
    return entry.device;
  case fromColumn:
    return entry.from;
  case toColumn:
    return entry.to;
  default:
    return {};
  }
}

QVariant AlertLogModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
    return {};
  }
  switch (section) {
  case timeColumn:
    return QStringLiteral("Time");
  case deviceColumn:
    return QStringLiteral("Device");
  case fromColumn:
    return QStringLiteral("From");
  case toColumn:
    return QStringLiteral("To");
  default:
    return {};
  }
}

void AlertLogModel::recordStatus(const QString &groupId,
                                 const messages::status_message::Status &status,
                                 const QDateTime &received) {
  const auto last      = m_lastState.constFind(groupId);
  const bool firstSeen = last == m_lastState.cend();
  if (!firstSeen && last.value() == status.state) {
    return; // no transition — nothing to log
  }

  const QString fromText = firstSeen ? QStringLiteral("—") : messages::status_message::stateName(last.value());
  const int row          = static_cast<int>(m_entries.size());
  beginInsertRows(QModelIndex(), row, row);
  m_entries.append(Entry{
      .time    = status.state == messages::status_message::State::Offline ? received : status.timestamp,
      .groupId = groupId,
      .device  = status.deviceId.isEmpty() ? groupId : status.deviceId,
      .from    = fromText,
      .to      = messages::status_message::stateName(status.state),
  });
  endInsertRows();
  m_lastState.insert(groupId, status.state);
  enforceCapacity();
}

void AlertLogModel::clear() {
  beginResetModel();
  m_entries.clear();
  m_lastState.clear();
  endResetModel();
}

void AlertLogModel::setCapacity(int rows) {
  m_capacity = rows;
  enforceCapacity();
}

void AlertLogModel::enforceCapacity() {
  if (m_capacity <= 0 || m_entries.size() <= m_capacity) {
    return;
  }
  const int excess = static_cast<int>(m_entries.size()) - m_capacity;
  beginRemoveRows(QModelIndex(), 0, excess - 1);
  m_entries.remove(0, excess);
  endRemoveRows();
}

} // namespace warden::core
