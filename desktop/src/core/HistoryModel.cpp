#include "core/HistoryModel.h"

#include <QString>

namespace warden::core {

HistoryModel::HistoryModel(QObject *parent)
      : QAbstractTableModel(parent) {}

int HistoryModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
}

int HistoryModel::columnCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : columnTotal; }

QVariant HistoryModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
    return {};
  }
  const Entry &entry = m_entries.at(index.row());

  if (role != Qt::DisplayRole) {
    return {};
  }
  switch (index.column()) {
  case timeColumn:
    return entry.time.toString(Qt::ISODate);
  case temperatureColumn:
    return messages::observation::format(entry.temperature, entry.temperatureUnit);
  case humidityColumn:
    return messages::observation::format(entry.humidity, entry.humidityUnit);
  default:
    return {};
  }
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
    return {};
  }
  switch (section) {
  case timeColumn:
    return QStringLiteral("Time");
  case temperatureColumn:
    return QStringLiteral("Temperature");
  case humidityColumn:
    return QStringLiteral("Humidity");
  default:
    return {};
  }
}

void HistoryModel::apply(Entry &entry, const messages::observation::Reading &reading) {
  switch (reading.metric) {
  case messages::observation::Metric::Temperature:
    entry.temperature     = reading.value;
    entry.temperatureUnit = reading.unit;
    break;
  case messages::observation::Metric::Humidity:
    entry.humidity     = reading.value;
    entry.humidityUnit = reading.unit;
    break;
  }
}

void HistoryModel::addReading(const messages::observation::Reading &reading) {
  // A measurement cycle publishes temperature and humidity with the same timestamp; merge
  // those into one row. Readings of a cycle arrive together, so only the last row can match.
  if (!m_entries.isEmpty() && m_entries.last().time == reading.effectiveTime) {
    apply(m_entries.last(), reading);
    const int row = static_cast<int>(m_entries.size()) - 1;
    emit dataChanged(index(row, 0), index(row, columnTotal - 1));
    return;
  }

  const int row = static_cast<int>(m_entries.size());
  beginInsertRows(QModelIndex(), row, row);
  Entry entry;
  entry.time = reading.effectiveTime;
  apply(entry, reading);
  m_entries.append(entry);
  endInsertRows();
  enforceCapacity();
}

void HistoryModel::clear() {
  if (m_entries.isEmpty()) {
    return;
  }
  beginResetModel();
  m_entries.clear();
  endResetModel();
}

void HistoryModel::setCapacity(int rows) {
  m_capacity = rows;
  enforceCapacity();
}

void HistoryModel::enforceCapacity() {
  if (m_capacity <= 0 || m_entries.size() <= m_capacity) {
    return;
  }
  const int excess = static_cast<int>(m_entries.size()) - m_capacity;
  beginRemoveRows(QModelIndex(), 0, excess - 1);
  m_entries.remove(0, excess);
  endRemoveRows();
}

} // namespace warden::core
