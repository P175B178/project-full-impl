#pragma once

#include "core/DeviceState.h"
#include "messages/Observation.h"
#include "messages/StatusMessage.h"

#include <QAbstractTableModel>
#include <QList>
#include <QModelIndex>
#include <QSet>
#include <QString>
#include <QStringList>
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
  static constexpr int stateColumn       = 3;
  static constexpr int connectionColumn  = 4; // online / offline
  static constexpr int columnTotal       = 5; // sentinel: number of columns

  /// Custom data role (int): the device's `messages::status_message::State` in the state column,
  /// for a view that shows it in color; no value until the device has reported a status.
  static constexpr int stateRole = Qt::UserRole + 1;

  /// Custom data role: what a view sorts a column by. A reading is a number here, where
  /// Qt::DisplayRole gives the text "9.5 °C", which would sort after "10.0 °C".
  static constexpr int sortRole = Qt::UserRole + 2;

  explicit DeviceModel(QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant
  headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  /// Route a parsed reading to its device, inserting a row if the group is new.
  void applyReading(const QString &groupId, const messages::observation::Reading &reading);

  /// Route a parsed status message to its device, inserting a row if the group is new.
  void applyStatus(const QString &groupId, const messages::status_message::Status &status);

  /// Forget every device, for a new source of device data.
  void clear();

  /// Look up a device by its group id, or nullptr if it has not been discovered.
  [[nodiscard]] const DeviceState *device(const QString &groupId) const;

  /// The device backing a given row. The row must be in range.
  [[nodiscard]] const DeviceState &deviceAt(int row) const;

  /// Group ids of the devices the user has ticked.
  [[nodiscard]] QStringList checkedGroups() const;

signals:
  /// Emitted whenever the set of ticked devices changes.
  void checkedDevicesChanged();

private:
  [[nodiscard]] int rowFor(const QString &groupId) const; // -1 if unknown
  int ensureRow(const QString &groupId);                  // returns the row, inserting if new
  void emitRowChanged(int row);

  QList<DeviceState> m_devices;
  QSet<QString> m_checked; // group ids of the ticked devices
};

} // namespace warden::core
