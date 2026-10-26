#pragma once

#include "messages/Observation.h"

#include <QColor>
#include <QHash>
#include <QString>
#include <QWidget>

class QChart;
class QDateTimeAxis;
class QLineSeries;
class QXYSeries;

namespace warden::app {

/// Live trend charts of temperature and humidity, one line per device in a stable
/// color. The time axis is a sliding window; older points are trimmed.
class ChartPanel : public QWidget {
  Q_OBJECT

public:
  explicit ChartPanel(QWidget *parent = nullptr);

  /// Append one reading to the matching device/metric series and slide the window forward.
  void addReading(const QString &groupId, const messages::observation::Reading &reading);

private:
  QLineSeries *seriesFor(const QString &groupId, messages::observation::Metric metric, const QString &name);
  QColor colorFor(const QString &groupId);
  void trim(QXYSeries *series) const;
  void slideWindow();

  QChart *m_temperatureChart;
  QChart *m_humidityChart;
  QDateTimeAxis *m_temperatureTimeAxis;
  QDateTimeAxis *m_humidityTimeAxis;
  QHash<QString, QLineSeries *> m_temperatureSeries;
  QHash<QString, QLineSeries *> m_humiditySeries;
  QHash<QString, QColor> m_colors;
  qint64 m_latestMs;
  int m_windowSeconds;
};

} // namespace warden::app
