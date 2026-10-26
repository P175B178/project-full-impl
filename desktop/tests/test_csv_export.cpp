#include "core/CsvExport.h"

#include <gtest/gtest.h>

#include <QFile>
#include <QStandardItemModel>
#include <QString>
#include <QTemporaryDir>

using namespace warden::core;

namespace {

// The bytes as written: no line-ending conversion.
QString readAll(const QString &path) {
  QFile file{path};
  EXPECT_TRUE(file.open(QIODevice::ReadOnly));
  return QString::fromUtf8(file.readAll());
}

} // namespace

TEST(CsvExportTest, WritesTheHeaderAndEveryRow) {
  QStandardItemModel model(2, 2);
  model.setHorizontalHeaderLabels({"Time", "Temperature"});
  model.setData(model.index(0, 0), "2026-01-02T03:04:05Z");
  model.setData(model.index(0, 1), QString::fromUtf8("23.4 °C"));
  model.setData(model.index(1, 0), "2026-01-02T03:04:10Z");
  model.setData(model.index(1, 1), QString::fromUtf8("23.5 °C"));
  const QTemporaryDir dir;
  const QString path = dir.filePath("history.csv");

  EXPECT_EQ(csv_export::writeModel(&model, path), QString());

  EXPECT_EQ(readAll(path), QString::fromUtf8("Time,Temperature\r\n"
                                             "2026-01-02T03:04:05Z,23.4 °C\r\n"
                                             "2026-01-02T03:04:10Z,23.5 °C\r\n"));
}

TEST(CsvExportTest, QuotesFieldsWithCommasQuotesOrLineBreaks) {
  QStandardItemModel model(1, 4);
  model.setHorizontalHeaderLabels({"a", "b", "c", "d"});
  model.setData(model.index(0, 0), "one, two");
  model.setData(model.index(0, 1), "say \"hi\"");
  model.setData(model.index(0, 2), "two\nlines");
  model.setData(model.index(0, 3), "carriage\rreturn");
  const QTemporaryDir dir;
  const QString path = dir.filePath("quoted.csv");

  EXPECT_EQ(csv_export::writeModel(&model, path), QString());

  EXPECT_EQ(readAll(path),
            QString("a,b,c,d\r\n\"one, two\",\"say \"\"hi\"\"\",\"two\nlines\",\"carriage\rreturn\"\r\n"));
}

TEST(CsvExportTest, ReportsAFileThatCannotBeWritten) {
  QStandardItemModel model(1, 1);

  EXPECT_FALSE(csv_export::writeModel(&model, "/nonexistent-folder/history.csv").isEmpty());
}
