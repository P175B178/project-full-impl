#pragma once

#include "core/DeviceState.h"
#include "messages/Observation.h"

#include <QAbstractTableModel>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QVariant>

namespace warden::core {

/// Every discovered device, one row each.
class DeviceModel : public QAbstractTableModel {
  Q_OBJECT

public:
  // Column layout of the device table.
  static constexpr int deviceColumn      = 0; // display name
  static constexpr int temperatureColumn = 1;
  static constexpr int humidityColumn    = 2;
  static constexpr int columnTotal       = 3; // sentinel: number of columns

  explicit DeviceModel(QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant
  headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  /// Route a parsed reading to its device, inserting a row if the group is new.
  void applyReading(const QString &groupId, const messages::observation::Reading &reading);

  /// Forget every device, for a new source of device data.
  void clear();

private:
  [[nodiscard]] int rowFor(const QString &groupId) const; // -1 if unknown
  int ensureRow(const QString &groupId);                  // returns the row, inserting if new
  void emitRowChanged(int row);

  QList<DeviceState> m_devices;
};

} // namespace warden::core
