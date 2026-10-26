#pragma once

#include "messages/StatusMessage.h"

#include <QAbstractTableModel>
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QVariant>

namespace warden::core {

/// Log of every device's state changes. A status that repeats the last state is left out.
/// Capped, dropping the oldest rows.
class AlertLogModel : public QAbstractTableModel {
  Q_OBJECT

public:
  static constexpr int timeColumn   = 0;
  static constexpr int deviceColumn = 1;
  static constexpr int fromColumn   = 2;
  static constexpr int toColumn     = 3;
  static constexpr int columnTotal  = 4; // sentinel: number of columns

  /// Custom data role (QString): the row's device group id, for filtering by device.
  static constexpr int groupIdRole = Qt::UserRole + 1;

  /// Default row cap: a busy class's changes over a long session.
  static constexpr int defaultCapacity = 1000;

  explicit AlertLogModel(QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant
  headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  /// Appends a row when the state differs from the device's last one (always, for its first).
  /// Offline shows `received`, not its own time: a Last Will carries the time the device connected.
  void recordStatus(const QString &groupId, const messages::status_message::Status &status, const QDateTime &received);

  /// Forgets everything, for a new source of device data.
  void clear();

  [[nodiscard]] int capacity() const { return m_capacity; }
  void setCapacity(int rows);

private:
  struct Entry {
    QDateTime time;
    QString groupId;
    QString device;
    QString from;
    QString to;
  };

  void enforceCapacity();

  QList<Entry> m_entries;
  QHash<QString, messages::status_message::State> m_lastState;
  int m_capacity = defaultCapacity;
};

} // namespace warden::core
