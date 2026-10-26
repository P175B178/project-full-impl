#include "core/DeviceModel.h"
#include "messages/Observation.h"

#include <gtest/gtest.h>

#include <QAbstractItemModelTester>
#include <QDate>
#include <QDateTime>
#include <QModelIndex>
#include <QString>
#include <QTime>
#include <QTimeZone>

using namespace warden::core;
using namespace warden::messages;

namespace {

// A model wrapped in the framework tester: every signal the model emits while these tests
// mutate it is checked for Model-View consistency, and any violation aborts the run.
struct TestedModel {
  DeviceModel model;
  QAbstractItemModelTester tester{&model, QAbstractItemModelTester::FailureReportingMode::Fatal};
};

observation::Reading temperatureReading(const QString &deviceId, double celsius) {
  return observation::Reading{
      .deviceId      = deviceId,
      .metric        = observation::Metric::Temperature,
      .value         = celsius,
      .unit          = QString::fromUtf8("°C"),
      .effectiveTime = QDateTime(QDate(2026, 1, 2), QTime(3, 4, 5), QTimeZone::UTC),
  };
}

observation::Reading humidityReading(const QString &deviceId, double percent) {
  return observation::Reading{
      .deviceId      = deviceId,
      .metric        = observation::Metric::Humidity,
      .value         = percent,
      .unit          = QStringLiteral("%"),
      .effectiveTime = QDateTime(QDate(2026, 1, 2), QTime(3, 4, 6), QTimeZone::UTC),
  };
}

QString cell(const DeviceModel &model, int row, int column) { return model.data(model.index(row, column)).toString(); }

} // namespace

TEST(DeviceModelTest, EmptyModelHasColumnsButNoRows) {
  TestedModel fixture;

  EXPECT_EQ(fixture.model.rowCount(), 0);
  EXPECT_EQ(fixture.model.columnCount(), DeviceModel::columnTotal);
}

TEST(DeviceModelTest, FirstReadingInsertsRow) {
  TestedModel fixture;

  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));

  ASSERT_EQ(fixture.model.rowCount(), 1);
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::deviceColumn), "group-1-pi5");
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::temperatureColumn), "23.5 °C");
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::humidityColumn), "—");
}

TEST(DeviceModelTest, SecondReadingForSameGroupUpdatesInPlace) {
  TestedModel fixture;

  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));
  fixture.model.applyReading("group-1", humidityReading("group-1-pi5", 45.0));

  ASSERT_EQ(fixture.model.rowCount(), 1);
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::temperatureColumn), "23.5 °C");
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::humidityColumn), "45.0 %");
}

TEST(DeviceModelTest, LaterReadingOverwritesTheMetric) {
  TestedModel fixture;

  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));
  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 19.0));

  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::temperatureColumn), "19.0 °C");
}

TEST(DeviceModelTest, DistinctGroupsInsertRowsInDiscoveryOrder) {
  TestedModel fixture;

  fixture.model.applyReading("group-2", temperatureReading("group-2-pi5", 21.0));
  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));

  ASSERT_EQ(fixture.model.rowCount(), 2);
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::deviceColumn), "group-2-pi5");
  EXPECT_EQ(cell(fixture.model, 1, DeviceModel::deviceColumn), "group-1-pi5");
}

TEST(DeviceModelTest, ClearForgetsEveryDevice) {
  TestedModel fixture;
  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));
  fixture.model.applyReading("group-2", temperatureReading("group-2-pi5", 21.0));

  fixture.model.clear();

  EXPECT_EQ(fixture.model.rowCount(), 0);
  fixture.model.applyReading("group-2", temperatureReading("group-2-pi5", 21.0));
  ASSERT_EQ(fixture.model.rowCount(), 1);
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::deviceColumn), "group-2-pi5");
}

TEST(DeviceModelTest, HeaderLabelsEveryColumn) {
  TestedModel fixture;

  EXPECT_EQ(fixture.model.headerData(DeviceModel::deviceColumn, Qt::Horizontal).toString(), "Device");
  EXPECT_EQ(fixture.model.headerData(DeviceModel::temperatureColumn, Qt::Horizontal).toString(), "Temperature");
  EXPECT_EQ(fixture.model.headerData(DeviceModel::humidityColumn, Qt::Horizontal).toString(), "Humidity");
}

TEST(DeviceModelTest, DataOutsideRangeIsInvalid) {
  TestedModel fixture;
  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));

  EXPECT_FALSE(fixture.model.data(fixture.model.index(0, DeviceModel::deviceColumn), Qt::DecorationRole).isValid());
  EXPECT_FALSE(fixture.model.data(QModelIndex()).isValid());
}
