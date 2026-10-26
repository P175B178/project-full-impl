#pragma once

#include <QPlainTextEdit>

class QString;

namespace warden::app {

/// The Log dock: one timestamped line per event, capped so a long session stays bounded.
class LogPanel : public QPlainTextEdit {
  Q_OBJECT

public:
  explicit LogPanel(QWidget *parent = nullptr);

  void appendMessage(const QString &message);
};

} // namespace warden::app
