#include "core/CsvExport.h"

#include <QAbstractItemModel>
#include <QLatin1Char>
#include <QLatin1StringView>
#include <QSaveFile>
#include <QStringList>
#include <QTextStream>

namespace warden::core::csv_export {

namespace {

// RFC 4180 ends every record with CRLF.
constexpr QLatin1StringView lineEnd{"\r\n"};

// Quote a CSV field when it contains a comma, a quote, or a line break (RFC 4180).
QString csvField(const QString &value) {
  if (value.contains(QLatin1Char(',')) || value.contains(QLatin1Char('"')) || value.contains(QLatin1Char('\n')) ||
      value.contains(QLatin1Char('\r'))) {
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QStringLiteral("\"%1\"").arg(escaped);
  }
  return value;
}

} // namespace

QString writeModel(const QAbstractItemModel *model, const QString &path) {
  QSaveFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    return file.errorString();
  }
  QTextStream out(&file);
  const int columns = model->columnCount();
  const int rows    = model->rowCount();

  QStringList header;
  header.reserve(columns);
  for (int column = 0; column < columns; ++column) {
    header.append(csvField(model->headerData(column, Qt::Horizontal).toString()));
  }
  out << header.join(QLatin1Char(',')) << lineEnd;

  for (int row = 0; row < rows; ++row) {
    QStringList cells;
    cells.reserve(columns);
    for (int column = 0; column < columns; ++column) {
      cells.append(csvField(model->data(model->index(row, column), Qt::EditRole).toString()));
    }
    out << cells.join(QLatin1Char(',')) << lineEnd;
  }

  out.flush();
  return file.commit() ? QString() : file.errorString();
}

} // namespace warden::core::csv_export
