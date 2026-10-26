#include "app/LogPanel.h"

#include <QDateTime>

namespace warden::app {

namespace {

constexpr int maxLines = 1000;

} // namespace

LogPanel::LogPanel(QWidget *parent)
      : QPlainTextEdit(parent) {
  setReadOnly(true);
  setMaximumBlockCount(maxLines);
}

void LogPanel::appendMessage(const QString &message) {
  // UTC, ending in Z, like every other time the console shows.
  const QString time = QDateTime::currentDateTimeUtc().toString(QStringLiteral("HH:mm:ss'Z'"));
  appendPlainText(QStringLiteral("%1  %2").arg(time, message));
}

} // namespace warden::app
