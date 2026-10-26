#pragma once

#include <QChartView>
#include <QPoint>

class QChart;
class QMouseEvent;
class QWheelEvent;

namespace warden::app {

/// A QChartView that zooms and pans along the time axis: drag or wheel to zoom, right-drag to pan,
/// double-click to reset. It signals these, so the owner can pause auto-scrolling.
class ZoomableChartView : public QChartView {
  Q_OBJECT

public:
  explicit ZoomableChartView(QChart *chart, QWidget *parent = nullptr);

signals:
  void viewChanged(); // the user zoomed or panned
  void viewReset();   // the user reset to the default view

protected:
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
  bool m_panning = false;
  QPoint m_lastPos;
  QPoint m_pressPos;
};

} // namespace warden::app
