#include "app/ZoomableChartView.h"

#include <QChart>
#include <QDateTime>
#include <QDateTimeAxis>
#include <QMouseEvent>
#include <QTimeZone>
#include <QWheelEvent>

namespace warden::app {

namespace {

constexpr double spanZoomIn  = 0.8;  // wheel up → shrink the visible time span (zoom in)
constexpr double spanZoomOut = 1.25; // wheel down → grow it (zoom out)
constexpr int dragThreshold  = 3;    // pixels — below this a left release is a click, not a zoom

// The chart's time axis, or nullptr. Zoom and pan act on it only: the Y range stays fixed.
QDateTimeAxis *horizontalTimeAxis(QChart *chart) {
  if (chart == nullptr || chart->axes(Qt::Horizontal).isEmpty()) {
    return nullptr;
  }
  return qobject_cast<QDateTimeAxis *>(chart->axes(Qt::Horizontal).constFirst());
}

} // namespace

ZoomableChartView::ZoomableChartView(QChart *chart, QWidget *parent)
      : QChartView(chart, parent) {
  // A left drag selects a time range to zoom into.
  setRubberBand(QChartView::HorizontalRubberBand);
}

void ZoomableChartView::wheelEvent(QWheelEvent *event) {
  QDateTimeAxis *axis = horizontalTimeAxis(chart());
  if (axis == nullptr) {
    QChartView::wheelEvent(event);
    return;
  }
  // Zoom about the time under the pointer: it stays where it is, the span around it shrinks
  // or grows.
  const double minMs    = static_cast<double>(axis->min().toMSecsSinceEpoch());
  const double maxMs    = static_cast<double>(axis->max().toMSecsSinceEpoch());
  const double cursorMs = chart()->mapToValue(chart()->mapFromScene(mapToScene(event->position().toPoint()))).x();
  const double factor   = event->angleDelta().y() > 0 ? spanZoomIn : spanZoomOut;
  const auto newMin     = static_cast<qint64>(cursorMs - ((cursorMs - minMs) * factor));
  const auto newMax     = static_cast<qint64>(cursorMs + ((maxMs - cursorMs) * factor));
  axis->setRange(QDateTime::fromMSecsSinceEpoch(newMin, QTimeZone::UTC),
                 QDateTime::fromMSecsSinceEpoch(newMax, QTimeZone::UTC));
  emit viewChanged();
  event->accept();
}

void ZoomableChartView::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::RightButton) {
    m_panning = true;
    m_lastPos = event->pos();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
    return;
  }
  m_pressPos = event->pos();
  QChartView::mousePressEvent(event); // left button starts the rubber band
}

void ZoomableChartView::mouseMoveEvent(QMouseEvent *event) {
  if (m_panning && chart() != nullptr) {
    // Pan along time only; the Y axis stays put.
    chart()->scroll(static_cast<qreal>(m_lastPos.x() - event->pos().x()), 0);
    m_lastPos = event->pos();
    emit viewChanged();
    event->accept();
    return;
  }
  QChartView::mouseMoveEvent(event);
}

void ZoomableChartView::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::RightButton && m_panning) {
    m_panning = false;
    unsetCursor();
    event->accept();
    return;
  }
  QChartView::mouseReleaseEvent(event); // applies the rubber-band time zoom
  if ((event->pos() - m_pressPos).manhattanLength() > dragThreshold) {
    emit viewChanged(); // a real drag-zoom, not a stray click
  }
}

void ZoomableChartView::mouseDoubleClickEvent(QMouseEvent *event) {
  if (chart() != nullptr) {
    chart()->zoomReset();
  }
  emit viewReset();
  event->accept();
}

} // namespace warden::app
