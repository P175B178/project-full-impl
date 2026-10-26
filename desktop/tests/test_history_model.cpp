#include "core/HistoryModel.h"
#include "messages/Observation.h"

#include <gtest/gtest.h>

#include <QAbstractItemModelTester>
#include <QDate>
#include <QDateTime>
#include <QModelIndex>
#include <QString>
#include <QTime>
#include <QTimeZone>

#include <optional>

using namespace warden::core;
using namespace warden::messages;

namespace {

struct TestedModel {
  HistoryModel model;
  QAbstractItemModelTester tester{&model, QAbstractItemModelTester::FailureReportingMode::Fatal};
};

QDateTime at(int second) { return {QDate(2026, 1, 2), QTime(3, 4, second), QTimeZone::UTC}; }

observation::Reading temperature(int second, double celsius) {
  return observation::Reading{
      .deviceId      = "group-1-pi5",
      .metric        = observation::Metric::Temperature,
      .value         = celsius,
      .unit          = QString::fromUtf8("°C"),
      .effectiveTime = at(second),
  };
}

observation::Reading humidity(int second, double percent) {
  return observation::Reading{
      .deviceId      = "group-1-pi5",
      .metric        = observation::Metric::Humidity,
      .value         = percent,
      .unit          = QStringLiteral("%"),
      .effectiveTime = at(second),
  };
}

QString cell(const HistoryModel &model, int row, int column) { return model.data(model.index(row, column)).toString(); }

bool violates(const HistoryModel &model, int row, int column) {
  return model.data(model.index(row, column), HistoryModel::violationRole).toBool();
}

} // namespace

TEST(HistoryModelTest, EmptyModelHasColumnsButNoRows) {
  TestedModel fixture;

  EXPECT_EQ(fixture.model.rowCount(), 0);
  EXPECT_EQ(fixture.model.columnCount(), HistoryModel::columnTotal);
}

TEST(HistoryModelTest, MergesSameTimestampIntoOneRow) {
  TestedModel fixture;

  fixture.model.addReading(temperature(1, 23.5));
  fixture.model.addReading(humidity(1, 45.0));

  ASSERT_EQ(fixture.model.rowCount(), 1);
  EXPECT_EQ(cell(fixture.model, 0, HistoryModel::temperatureColumn), "23.5 °C");
  EXPECT_EQ(cell(fixture.model, 0, HistoryModel::humidityColumn), "45.0 %");
}

TEST(HistoryModelTest, DistinctTimestampsAreSeparateRows) {
  TestedModel fixture;

  fixture.model.addReading(temperature(1, 23.5));
  fixture.model.addReading(temperature(2, 24.0));

  ASSERT_EQ(fixture.model.rowCount(), 2);
  EXPECT_EQ(cell(fixture.model, 1, HistoryModel::temperatureColumn), "24.0 °C");
}

TEST(HistoryModelTest, MissingMetricShowsDash) {
  TestedModel fixture;

  fixture.model.addReading(temperature(1, 23.5)); // no humidity this row

  EXPECT_EQ(cell(fixture.model, 0, HistoryModel::humidityColumn), "—");
}

TEST(HistoryModelTest, NoThresholdMeansNoViolation) {
  TestedModel fixture;
  fixture.model.addReading(temperature(1, 99.0));

  EXPECT_FALSE(violates(fixture.model, 0, HistoryModel::temperatureColumn));
}

TEST(HistoryModelTest, FlagsOnlyTheBreachingMetricCell) {
  TestedModel fixture;
  fixture.model.setTemperatureThreshold(30.0);
  fixture.model.setHumidityThreshold(80.0);

  fixture.model.addReading(temperature(1, 35.0)); // over temp threshold
  fixture.model.addReading(humidity(1, 45.0));    // under humidity threshold

  EXPECT_TRUE(violates(fixture.model, 0, HistoryModel::temperatureColumn));
  EXPECT_FALSE(violates(fixture.model, 0, HistoryModel::humidityColumn));
  EXPECT_FALSE(violates(fixture.model, 0, HistoryModel::timeColumn));
}

TEST(HistoryModelTest, ChangingThresholdReevaluatesExistingRows) {
  TestedModel fixture;
  fixture.model.addReading(temperature(1, 28.0));

  EXPECT_FALSE(violates(fixture.model, 0, HistoryModel::temperatureColumn));

  fixture.model.setTemperatureThreshold(25.0);
  EXPECT_TRUE(violates(fixture.model, 0, HistoryModel::temperatureColumn));

  fixture.model.setTemperatureThreshold(std::nullopt);
  EXPECT_FALSE(violates(fixture.model, 0, HistoryModel::temperatureColumn));
}

TEST(HistoryModelTest, ValueEqualToThresholdIsNotAViolation) {
  TestedModel fixture;
  fixture.model.setTemperatureThreshold(30.0);

  fixture.model.addReading(temperature(1, 30.0));

  EXPECT_FALSE(violates(fixture.model, 0, HistoryModel::temperatureColumn));
}

TEST(HistoryModelTest, CapacityDropsOldestRows) {
  TestedModel fixture;
  fixture.model.setCapacity(2);

  fixture.model.addReading(temperature(1, 10.0));
  fixture.model.addReading(temperature(2, 20.0));
  fixture.model.addReading(temperature(3, 30.0));

  ASSERT_EQ(fixture.model.rowCount(), 2);
  EXPECT_EQ(cell(fixture.model, 0, HistoryModel::temperatureColumn), "20.0 °C");
  EXPECT_EQ(cell(fixture.model, 1, HistoryModel::temperatureColumn), "30.0 °C");
}

TEST(HistoryModelTest, ClearEmptiesHistory) {
  TestedModel fixture;
  fixture.model.addReading(temperature(1, 10.0));

  fixture.model.clear();

  EXPECT_EQ(fixture.model.rowCount(), 0);
}

TEST(HistoryModelTest, HeaderLabelsEveryColumn) {
  TestedModel fixture;

  EXPECT_EQ(fixture.model.headerData(HistoryModel::timeColumn, Qt::Horizontal).toString(), "Time");
  EXPECT_EQ(fixture.model.headerData(HistoryModel::temperatureColumn, Qt::Horizontal).toString(), "Temperature");
  EXPECT_EQ(fixture.model.headerData(HistoryModel::humidityColumn, Qt::Horizontal).toString(), "Humidity");
}

TEST(HistoryModelTest, EditRoleGivesTheRawValues) {
  TestedModel fixture;

  fixture.model.addReading(temperature(1, 23.5));

  const auto raw = [&fixture](int column) { return fixture.model.data(fixture.model.index(0, column), Qt::EditRole); };
  EXPECT_EQ(raw(HistoryModel::timeColumn).toString(), "2026-01-02T03:04:01Z");
  EXPECT_EQ(raw(HistoryModel::temperatureColumn), QVariant(23.5));
  EXPECT_FALSE(raw(HistoryModel::humidityColumn).isValid()); // no humidity reading yet
}
