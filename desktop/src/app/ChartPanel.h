#pragma once

#include "messages/Observation.h"

#include <QColor>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QWidget>

class QChart;
class QDateTimeAxis;
class QLabel;
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

  /// Show only the given devices' series; others are hidden. New series for a hidden device
  /// start hidden too.
  void setVisibleDevices(const QStringList &groupIds);

  /// Remove every device's series, for a new source of device data.
  void clear();

private:
  QLineSeries *seriesFor(const QString &groupId, messages::observation::Metric metric, const QString &name);
  QColor colorFor(const QString &groupId);
  void trim(QXYSeries *series) const;
  void slideWindow();

  QChart *m_temperatureChart;
  QChart *m_humidityChart;
  QDateTimeAxis *m_temperatureTimeAxis;
  QDateTimeAxis *m_humidityTimeAxis;
  QLabel *m_pauseHint; // shown while auto-scroll is paused
  QHash<QString, QLineSeries *> m_temperatureSeries;
  QHash<QString, QLineSeries *> m_humiditySeries;
  QHash<QString, QColor> m_colors;
  QSet<QString> m_visibleDevices; // devices whose series are shown
  qint64 m_latestMs;
  int m_windowSeconds;
  // Off while the user has zoomed or panned.
  bool m_autoScroll = true;
};

} // namespace warden::app
