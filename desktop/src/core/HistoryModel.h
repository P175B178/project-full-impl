#pragma once

#include "messages/Observation.h"

#include <QAbstractTableModel>
#include <QDateTime>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QVariant>

#include <optional>

namespace warden::core {

/// History of one device's readings, one row per measurement cycle: temperature and humidity
/// with the same timestamp are merged. Capped, dropping the oldest rows.
class HistoryModel : public QAbstractTableModel {
  Q_OBJECT

public:
  static constexpr int timeColumn        = 0;
  static constexpr int temperatureColumn = 1;
  static constexpr int humidityColumn    = 2;
  static constexpr int columnTotal       = 3; // sentinel: number of columns

  // Qt::DisplayRole gives a cell as shown, "23.4 °C"; Qt::EditRole gives its raw value, 23.4,
  // for an export.

  /// Custom data role (bool): true when this cell's value exceeds its metric's threshold.
  static constexpr int violationRole = Qt::UserRole + 1;

  /// Default row cap — an hour of measurement cycles at the shortest interval, 2 s.
  static constexpr int defaultCapacity = 1800;

  explicit HistoryModel(QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant
  headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  /// Merges the reading into its timestamp's row, or starts one, dropping the oldest when full.
  void addReading(const messages::observation::Reading &reading);

  /// Set or clear (nullopt) the thresholds used to flag violations. Existing rows are
  /// re-evaluated, so the highlight follows a new threshold.
  void setTemperatureThreshold(std::optional<double> celsius);
  void setHumidityThreshold(std::optional<double> percent);

  /// Drop all history.
  void clear();

  [[nodiscard]] int capacity() const { return m_capacity; }
  void setCapacity(int rows);

private:
  struct Entry {
    QDateTime time;
    std::optional<double> temperature;
    QString temperatureUnit;
    std::optional<double> humidity;
    QString humidityUnit;
  };

  static void apply(Entry &entry, const messages::observation::Reading &reading);
  [[nodiscard]] bool temperatureExceeds(const Entry &entry) const;
  [[nodiscard]] bool humidityExceeds(const Entry &entry) const;
  void enforceCapacity();
  void refreshViolations();

  QList<Entry> m_entries;
  std::optional<double> m_temperatureThreshold;
  std::optional<double> m_humidityThreshold;
  int m_capacity = defaultCapacity;
};

} // namespace warden::core
