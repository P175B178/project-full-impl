#include "app/ChartPanel.h"

#include "app/Colors.h"
#include "app/ZoomableChartView.h"

#include <QAbstractAxis>
#include <QChart>
#include <QDateTime>
#include <QDateTimeAxis>
#include <QGraphicsLayout>
#include <QLabel>
#include <QLegend>
#include <QLineSeries>
#include <QPainter>
#include <QTimeZone>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QXYSeries>

#include <algorithm>

namespace warden::app {

namespace {

constexpr int windowSecondsDefault = 300;
constexpr int msPerSecond          = 1000;
constexpr int secondsPerMinute     = 60;
constexpr qint64 msPerMinute       = static_cast<qint64>(secondsPerMinute) * msPerSecond;

// Below this visible span, label the time axis with seconds; above it, with minutes.
constexpr int secondsFormatThreshold = 120;
constexpr int maxTimeTicks           = 6;
constexpr int maxSecondTicks         = 4; // HH:mm:ssZ labels are wider

// Picks the label format and tick count for the visible span: QDateTimeAxis spaces labels evenly,
// so fixed minute labels would repeat when zoomed in close.
void adaptTimeAxis(QDateTimeAxis *axis) {
  const qint64 spanSeconds = axis->min().secsTo(axis->max());
  if (spanSeconds <= 0) {
    return;
  }
  if (spanSeconds <= secondsFormatThreshold) {
    axis->setFormat(QStringLiteral("HH:mm:ss'Z'"));
    axis->setTickCount(static_cast<int>(qMax<qint64>(2, qMin<qint64>(maxSecondTicks, spanSeconds + 1))));
  } else {
    axis->setFormat(QStringLiteral("HH:mm'Z'"));
    axis->setTickCount(
        static_cast<int>(qMax<qint64>(2, qMin<qint64>(maxTimeTicks, (spanSeconds / secondsPerMinute) + 1))));
  }
}

// Y ranges: the device's default plausible range (device/config/warden.toml).
constexpr double minTemperature = 0.0;
constexpr double maxTemperature = 50.0;
constexpr double minHumidity    = 0.0;
constexpr double maxHumidity    = 100.0;

QChart *makeChart(const QString &title) {
  auto *chart = new QChart;
  // Thin margins: a small chart needs the room for its axis labels.
  chart->layout()->setContentsMargins(0, 0, 0, 0);
  chart->setBackgroundRoundness(0);
  chart->setTitle(title);
  chart->legend()->setVisible(true);
  chart->legend()->setAlignment(Qt::AlignRight); // height is scarcer than width
  return chart;
}

void configureAxes(QChart *chart, QDateTimeAxis *timeAxis, double yMin, double yMax, const QString &yTitle) {
  // One tick per minute, on round HH:mm values (slideWindow() snaps to minutes). UTC, like every
  // time the console shows.
  timeAxis->setFormat(QStringLiteral("HH:mm'Z'"));
  timeAxis->setTickCount((windowSecondsDefault / secondsPerMinute) + 1);
  // No "Time" title: the clock times say it.
  auto *valueAxis = new QValueAxis;
  valueAxis->setRange(yMin, yMax);
  valueAxis->setTitleText(yTitle);
  chart->addAxis(timeAxis, Qt::AlignBottom);
  chart->addAxis(valueAxis, Qt::AlignLeft);
}

// The legend lists series in the order they were added, so all are added again, by name.
void addSortedByName(QChart *chart, QList<QLineSeries *> series) {
  std::ranges::sort(series, {}, &QLineSeries::name);
  QAbstractAxis *timeAxis  = chart->axes(Qt::Horizontal).constFirst();
  QAbstractAxis *valueAxis = chart->axes(Qt::Vertical).constFirst();
  for (QLineSeries *line : series) {
    if (line->chart() == chart) {
      chart->removeSeries(line);
    }
    chart->addSeries(line);
    line->attachAxis(timeAxis);
    line->attachAxis(valueAxis);
  }
}

} // namespace

ChartPanel::ChartPanel(QWidget *parent)
      : QWidget(parent)
      , m_temperatureChart(makeChart(QStringLiteral("Temperature")))
      , m_humidityChart(makeChart(QStringLiteral("Humidity")))
      , m_temperatureTimeAxis(new QDateTimeAxis)
      , m_humidityTimeAxis(new QDateTimeAxis)
      , m_latestMs(QDateTime::currentMSecsSinceEpoch())
      , m_windowSeconds(windowSecondsDefault) {
  configureAxes(m_temperatureChart, m_temperatureTimeAxis, minTemperature, maxTemperature, QStringLiteral("°C"));
  configureAxes(m_humidityChart, m_humidityTimeAxis, minHumidity, maxHumidity, QStringLiteral("%"));

  // The span changes live and on zoom; the labels follow.
  for (QDateTimeAxis *axis : {m_temperatureTimeAxis, m_humidityTimeAxis}) {
    connect(axis, &QDateTimeAxis::rangeChanged, this, [axis] { adaptTimeAxis(axis); });
  }

  auto *temperatureView = new ZoomableChartView(m_temperatureChart, this);
  temperatureView->setRenderHint(QPainter::Antialiasing);
  auto *humidityView = new ZoomableChartView(m_humidityChart, this);
  humidityView->setRenderHint(QPainter::Antialiasing);

  m_pauseHint = new QLabel(
      QStringLiteral("Paused — scroll/drag zooms time; double-click a chart to resume the live view."), this);
  m_pauseHint->setStyleSheet(
      QStringLiteral("color: %1; background: %2; padding: 2px 6px;").arg(hintText().name(), hintBackground().name()));
  m_pauseHint->setVisible(false);

  // Pause the sliding window while the user inspects a zoomed/panned view; resume on reset.
  for (ZoomableChartView *view : {temperatureView, humidityView}) {
    connect(view, &ZoomableChartView::viewChanged, this, [this] {
      m_autoScroll = false;
      m_pauseHint->setVisible(true);
    });
    connect(view, &ZoomableChartView::viewReset, this, [this] {
      m_autoScroll = true;
      m_pauseHint->setVisible(false);
      slideWindow();
    });
  }

  auto *layout = new QVBoxLayout(this);
  layout->addWidget(m_pauseHint);
  layout->addWidget(temperatureView);
  layout->addWidget(humidityView);

  slideWindow();
}

void ChartPanel::addReading(const QString &groupId, const messages::observation::Reading &reading) {
  const qint64 pointMs = reading.effectiveTime.toMSecsSinceEpoch();
  m_latestMs           = qMax(m_latestMs, pointMs);

  QLineSeries *series = seriesFor(groupId, reading.metric, reading.deviceId);
  series->append(static_cast<double>(pointMs), reading.value);
  trim(series);
  slideWindow();
}

void ChartPanel::setVisibleDevices(const QStringList &groupIds) {
  m_visibleDevices = QSet<QString>(groupIds.cbegin(), groupIds.cend());
  for (auto it = m_temperatureSeries.cbegin(); it != m_temperatureSeries.cend(); ++it) {
    it.value()->setVisible(m_visibleDevices.contains(it.key()));
  }
  for (auto it = m_humiditySeries.cbegin(); it != m_humiditySeries.cend(); ++it) {
    it.value()->setVisible(m_visibleDevices.contains(it.key()));
  }
}

void ChartPanel::clear() {
  for (QLineSeries *series : std::as_const(m_temperatureSeries)) {
    m_temperatureChart->removeSeries(series);
    delete series; // removeSeries hands ownership back
  }
  for (QLineSeries *series : std::as_const(m_humiditySeries)) {
    m_humidityChart->removeSeries(series);
    delete series; // removeSeries hands ownership back
  }
  m_temperatureSeries.clear();
  m_humiditySeries.clear();
  m_colors.clear();
  m_latestMs = QDateTime::currentMSecsSinceEpoch();
}

void ChartPanel::trim(QXYSeries *series) const {
  // Drop points older than the window, so the series stays bounded.
  const auto cutoff = static_cast<double>(m_latestMs - (static_cast<qint64>(m_windowSeconds) * msPerSecond));
  int stale         = 0;
  while (stale < series->count() && series->at(stale).x() < cutoff) {
    ++stale;
  }
  if (stale > 0) {
    series->removePoints(0, stale);
  }
}

QLineSeries *ChartPanel::seriesFor(const QString &groupId, messages::observation::Metric metric, const QString &name) {
  const bool isTemperature             = metric == messages::observation::Metric::Temperature;
  QHash<QString, QLineSeries *> &table = isTemperature ? m_temperatureSeries : m_humiditySeries;
  const auto existing                  = table.constFind(groupId);
  if (existing != table.cend()) {
    return existing.value();
  }

  QChart *chart = isTemperature ? m_temperatureChart : m_humidityChart;
  auto *series  = new QLineSeries;
  series->setName(name);
  series->setColor(colorFor(groupId));
  series->setVisible(m_visibleDevices.contains(groupId));
  table.insert(groupId, series);
  addSortedByName(chart, table.values());
  return series;
}

QColor ChartPanel::colorFor(const QString &groupId) {
  const auto existing = m_colors.constFind(groupId);
  if (existing != m_colors.cend()) {
    return existing.value();
  }
  const QColor color = seriesColor(m_colors.size());
  m_colors.insert(groupId, color);
  return color;
}

void ChartPanel::slideWindow() {
  if (!m_autoScroll) {
    return; // keep the user's zoom
  }
  // Snap the right edge up to the next whole minute, so the ticks land on round HH:mm values.
  const qint64 end          = ((m_latestMs + msPerMinute - 1) / msPerMinute) * msPerMinute;
  const qint64 start        = end - (static_cast<qint64>(m_windowSeconds) * msPerSecond);
  const QDateTime startTime = QDateTime::fromMSecsSinceEpoch(start, QTimeZone::UTC);
  const QDateTime endTime   = QDateTime::fromMSecsSinceEpoch(end, QTimeZone::UTC);
  m_temperatureTimeAxis->setRange(startTime, endTime);
  m_humidityTimeAxis->setRange(startTime, endTime);
}

} // namespace warden::app
