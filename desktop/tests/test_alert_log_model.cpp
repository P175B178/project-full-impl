#include "core/AlertLogModel.h"
#include "messages/StatusMessage.h"

#include <gtest/gtest.h>

#include <QAbstractItemModelTester>
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QTimeZone>

using namespace warden::core;
using namespace warden::messages;

namespace {

struct TestedModel {
  AlertLogModel model;
  QAbstractItemModelTester tester{&model, QAbstractItemModelTester::FailureReportingMode::Fatal};
};

status_message::Status statusMessage(const QString &deviceId, status_message::State state, int second) {
  return status_message::Status{
      .deviceId  = deviceId,
      .state     = state,
      .timestamp = QDateTime(QDate(2026, 1, 2), QTime(0, 0, second), QTimeZone::UTC),
  };
}

// When the console got a status: an hour after any of the statuses' own times.
const QDateTime received{QDate(2026, 1, 2), QTime(1, 0, 0), QTimeZone::UTC};

QString cell(const AlertLogModel &model, int row, int column) {
  return model.data(model.index(row, column)).toString();
}

} // namespace

TEST(AlertLogModelTest, EmptyModelHasColumnsButNoRows) {
  TestedModel fixture;

  EXPECT_EQ(fixture.model.rowCount(), 0);
  EXPECT_EQ(fixture.model.columnCount(), AlertLogModel::columnTotal);
}

TEST(AlertLogModelTest, FirstStatusIsLoggedWithNoPriorState) {
  TestedModel fixture;

  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal, 1), received);

  ASSERT_EQ(fixture.model.rowCount(), 1);
  EXPECT_EQ(cell(fixture.model, 0, AlertLogModel::deviceColumn), "group-1-pi5");
  EXPECT_EQ(cell(fixture.model, 0, AlertLogModel::fromColumn), "—");
  EXPECT_EQ(cell(fixture.model, 0, AlertLogModel::toColumn), "Normal");
}

TEST(AlertLogModelTest, RepeatedStateIsNotLogged) {
  TestedModel fixture;

  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal, 1), received);
  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal, 2), received);

  EXPECT_EQ(fixture.model.rowCount(), 1);
}

TEST(AlertLogModelTest, TransitionRecordsFromAndTo) {
  TestedModel fixture;

  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal, 1), received);
  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Alert, 2), received);

  ASSERT_EQ(fixture.model.rowCount(), 2);
  EXPECT_EQ(cell(fixture.model, 1, AlertLogModel::fromColumn), "Normal");
  EXPECT_EQ(cell(fixture.model, 1, AlertLogModel::toColumn), "Alert");
}

TEST(AlertLogModelTest, FaultTransitionIsLogged) {
  TestedModel fixture;

  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal, 1), received);
  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Fault, 2), received);

  EXPECT_EQ(cell(fixture.model, 1, AlertLogModel::toColumn), "Fault");
}

TEST(AlertLogModelTest, OfflineTransitionIsLogged) {
  TestedModel fixture;

  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Warning, 1), received);
  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Offline, 2), received);

  EXPECT_EQ(cell(fixture.model, 1, AlertLogModel::toColumn), "Offline");
}

TEST(AlertLogModelTest, TransitionsAreTrackedPerDevice) {
  TestedModel fixture;

  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal, 1), received);
  fixture.model.recordStatus("group-2", statusMessage("group-2-pi5", status_message::State::Normal, 1), received);
  // group-2's Normal must not be suppressed by group-1's Normal.

  ASSERT_EQ(fixture.model.rowCount(), 2);
  EXPECT_EQ(cell(fixture.model, 1, AlertLogModel::deviceColumn), "group-2-pi5");
}

TEST(AlertLogModelTest, ExposesGroupIdForFiltering) {
  TestedModel fixture;

  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal, 1), received);

  EXPECT_EQ(
      fixture.model.data(fixture.model.index(0, AlertLogModel::timeColumn), AlertLogModel::groupIdRole).toString(),
      "group-1");
}

TEST(AlertLogModelTest, ClearForgetsTheLogAndTheLastStates) {
  TestedModel fixture;
  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal, 1), received);

  fixture.model.clear();

  EXPECT_EQ(fixture.model.rowCount(), 0);
  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal, 2), received);
  ASSERT_EQ(fixture.model.rowCount(), 1);
  EXPECT_EQ(cell(fixture.model, 0, AlertLogModel::fromColumn), "—");
}

TEST(AlertLogModelTest, CapacityDropsOldestRows) {
  TestedModel fixture;
  fixture.model.setCapacity(2);

  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Normal, 1), received);
  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Warning, 2), received);
  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Alert, 3), received);

  ASSERT_EQ(fixture.model.rowCount(), 2);
  EXPECT_EQ(cell(fixture.model, 0, AlertLogModel::toColumn), "Warning");
  EXPECT_EQ(cell(fixture.model, 1, AlertLogModel::toColumn), "Alert");
}

TEST(AlertLogModelTest, HeaderLabelsEveryColumn) {
  TestedModel fixture;

  EXPECT_EQ(fixture.model.headerData(AlertLogModel::timeColumn, Qt::Horizontal).toString(), "Time");
  EXPECT_EQ(fixture.model.headerData(AlertLogModel::deviceColumn, Qt::Horizontal).toString(), "Device");
  EXPECT_EQ(fixture.model.headerData(AlertLogModel::fromColumn, Qt::Horizontal).toString(), "From");
  EXPECT_EQ(fixture.model.headerData(AlertLogModel::toColumn, Qt::Horizontal).toString(), "To");
}

TEST(AlertLogModelTest, AStateChangeShowsTheTimeTheDeviceGave) {
  TestedModel fixture;

  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Warning, 7), received);

  EXPECT_EQ(cell(fixture.model, 0, AlertLogModel::timeColumn), "2026-01-02T00:00:07Z");
}

TEST(AlertLogModelTest, OfflineShowsWhenItArrived) {
  TestedModel fixture;

  // A Last Will carries the time the device connected, not the time it disappeared.
  fixture.model.recordStatus("group-1", statusMessage("group-1-pi5", status_message::State::Offline, 7), received);

  EXPECT_EQ(cell(fixture.model, 0, AlertLogModel::timeColumn), "2026-01-02T01:00:00Z");
}
