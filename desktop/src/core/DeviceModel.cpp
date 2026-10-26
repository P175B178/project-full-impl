#include "core/DeviceModel.h"

#include <QString>

namespace warden::core {

DeviceModel::DeviceModel(QObject *parent)
      : QAbstractTableModel(parent) {}

int DeviceModel::rowCount(const QModelIndex &parent) const {
  // A flat table exposes its rows only under the root; any valid index is a leaf.
  return parent.isValid() ? 0 : static_cast<int>(m_devices.size());
}

int DeviceModel::columnCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : columnTotal; }

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_devices.size()) {
    return {};
  }
  const DeviceState &device = m_devices.at(index.row());

  if (role == stateRole) {
    if (index.column() != stateColumn || !device.hasStatus) {
      return {};
    }
    return static_cast<int>(device.state);
  }
  if (role != Qt::DisplayRole) {
    return {};
  }
  switch (index.column()) {
  case deviceColumn:
    return device.displayName();
  case temperatureColumn:
    return messages::observation::format(device.temperature, device.temperatureUnit);
  case humidityColumn:
    return messages::observation::format(device.humidity, device.humidityUnit);
  case stateColumn:
    return device.hasStatus ? messages::status_message::stateName(device.state) : QStringLiteral("—");
  case connectionColumn:
    return device.online() ? QStringLiteral("Online") : QStringLiteral("Offline");
  default:
    return {};
  }
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
    return {};
  }
  switch (section) {
  case deviceColumn:
    return QStringLiteral("Device");
  case temperatureColumn:
    return QStringLiteral("Temperature");
  case humidityColumn:
    return QStringLiteral("Humidity");
  case stateColumn:
    return QStringLiteral("State");
  case connectionColumn:
    return QStringLiteral("Connection");
  default:
    return {};
  }
}

void DeviceModel::applyReading(const QString &groupId, const messages::observation::Reading &reading) {
  const int row = ensureRow(groupId);
  m_devices[row].apply(reading);
  emitRowChanged(row);
}

void DeviceModel::applyStatus(const QString &groupId, const messages::status_message::Status &status) {
  const int row = ensureRow(groupId);
  m_devices[row].apply(status);
  emitRowChanged(row);
}

void DeviceModel::clear() {
  beginResetModel();
  m_devices.clear();
  endResetModel();
}

// A linear search: a class has only a few devices.
int DeviceModel::rowFor(const QString &groupId) const {
  for (int row = 0; row < m_devices.size(); ++row) {
    if (m_devices.at(row).groupId == groupId) {
      return row;
    }
  }
  return -1;
}

int DeviceModel::ensureRow(const QString &groupId) {
  const int existing = rowFor(groupId);
  if (existing >= 0) {
    return existing;
  }
  const int row = static_cast<int>(m_devices.size());
  DeviceState device;
  device.groupId = groupId;
  beginInsertRows(QModelIndex(), row, row);
  m_devices.append(device);
  endInsertRows();
  return row;
}

void DeviceModel::emitRowChanged(int row) { emit dataChanged(index(row, 0), index(row, columnTotal - 1)); }

} // namespace warden::core
