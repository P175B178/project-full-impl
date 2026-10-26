#include "core/DeviceModel.h"
#include "messages/Observation.h"
#include "messages/StatusMessage.h"

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

status_message::Status statusMessage(const QString &deviceId, status_message::State state) {
  return status_message::Status{
      .deviceId  = deviceId,
      .state     = state,
      .timestamp = QDateTime(QDate(2026, 1, 2), QTime(3, 4, 7), QTimeZone::UTC),
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

TEST(DeviceModelTest, StateIsUnknownUntilTheFirstStatus) {
  TestedModel fixture;

  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));

  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::stateColumn), "—");
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::connectionColumn), "Online");
}

TEST(DeviceModelTest, StatusUpdatesStateAndKeepsDeviceOnline) {
  TestedModel fixture;

  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));
  fixture.model.applyStatus("group-1", statusMessage("group-1-pi5", status_message::State::Warning));

  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::stateColumn), "Warning");
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::connectionColumn), "Online");
}

TEST(DeviceModelTest, FaultKeepsDeviceOnline) {
  TestedModel fixture;

  fixture.model.applyStatus("group-1", statusMessage("group-1-pi5", status_message::State::Fault));

  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::stateColumn), "Fault");
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::connectionColumn), "Online");
}

TEST(DeviceModelTest, OfflineStatusMarksDeviceOffline) {
  TestedModel fixture;

  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));
  fixture.model.applyStatus("group-1", statusMessage("group-1-pi5", status_message::State::Offline));

  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::stateColumn), "Offline");
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::connectionColumn), "Offline");
}

TEST(DeviceModelTest, DeviceMayBeDiscoveredByStatusFirst) {
  TestedModel fixture;

  fixture.model.applyStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal));

  ASSERT_EQ(fixture.model.rowCount(), 1);
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::deviceColumn), "group-1-pi5");
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::temperatureColumn), "—");
  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::connectionColumn), "Online");
}

TEST(DeviceModelTest, DisplayNameFallsBackToGroupId) {
  TestedModel fixture;

  // A status with no display name still discovers the device; the table shows the group.
  fixture.model.applyStatus("group-1", statusMessage(QString(), status_message::State::Normal));

  EXPECT_EQ(cell(fixture.model, 0, DeviceModel::deviceColumn), "group-1");
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
  EXPECT_EQ(fixture.model.headerData(DeviceModel::stateColumn, Qt::Horizontal).toString(), "State");
  EXPECT_EQ(fixture.model.headerData(DeviceModel::connectionColumn, Qt::Horizontal).toString(), "Connection");
}

TEST(DeviceModelTest, LooksUpADeviceByGroup) {
  TestedModel fixture;
  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));

  const DeviceState *device = fixture.model.device("group-1");
  ASSERT_NE(device, nullptr);
  EXPECT_EQ(device->deviceId, "group-1-pi5");
  EXPECT_EQ(&fixture.model.deviceAt(0), device);
}

TEST(DeviceModelTest, LookupOfUnknownGroupReturnsNull) {
  TestedModel fixture;

  EXPECT_EQ(fixture.model.device("nope"), nullptr);
}

TEST(DeviceModelTest, DataOutsideRangeIsInvalid) {
  TestedModel fixture;
  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));

  EXPECT_FALSE(fixture.model.data(fixture.model.index(0, DeviceModel::deviceColumn), Qt::DecorationRole).isValid());
  EXPECT_FALSE(fixture.model.data(QModelIndex()).isValid());
}

TEST(DeviceModelTest, StateRoleGivesTheStateOnceItIsKnown) {
  TestedModel fixture;
  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 23.5));
  const QModelIndex stateCell = fixture.model.index(0, DeviceModel::stateColumn);

  EXPECT_FALSE(fixture.model.data(stateCell, DeviceModel::stateRole).isValid()); // readings only, so far

  fixture.model.applyStatus("group-1", statusMessage("group-1-pi5", status_message::State::Alert));

  EXPECT_EQ(fixture.model.data(stateCell, DeviceModel::stateRole).toInt(),
            static_cast<int>(status_message::State::Alert));
  // Only the state column carries it.
  EXPECT_FALSE(fixture.model.data(fixture.model.index(0, DeviceModel::deviceColumn), DeviceModel::stateRole).isValid());
}

TEST(DeviceModelTest, SortRoleGivesAReadingAsANumber) {
  TestedModel fixture;

  fixture.model.applyReading("group-1", temperatureReading("group-1-pi5", 9.5));

  const auto sortKey = [&fixture](int column) {
    return fixture.model.data(fixture.model.index(0, column), DeviceModel::sortRole);
  };
  EXPECT_EQ(sortKey(DeviceModel::temperatureColumn), QVariant(9.5));
  EXPECT_FALSE(sortKey(DeviceModel::humidityColumn).isValid()); // no humidity reading yet
  EXPECT_EQ(sortKey(DeviceModel::deviceColumn), QVariant("group-1-pi5"));
}
