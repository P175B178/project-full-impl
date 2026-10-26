#include "core/DeviceMonitor.h"
#include "core/HistoryModel.h"
#include "messages/Observation.h"

#include <gtest/gtest.h>

#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QTimeZone>

using namespace warden::core;
using namespace warden::messages;

namespace {

observation::Reading temperatureAt(int second, double celsius) {
  return observation::Reading{
      .deviceId      = "group-1-pi5",
      .metric        = observation::Metric::Temperature,
      .value         = celsius,
      .unit          = QString::fromUtf8("°C"),
      .effectiveTime = QDateTime(QDate(2026, 1, 2), QTime(0, 0, second), QTimeZone::UTC),
  };
}

} // namespace

TEST(DeviceMonitorTest, ReadingCreatesAndGrowsHistory) {
  DeviceMonitor monitor;

  monitor.addReading("group-1", temperatureAt(1, 20.0));
  monitor.addReading("group-1", temperatureAt(2, 20.5));

  EXPECT_EQ(monitor.historyFor("group-1")->rowCount(), 2);
}

TEST(DeviceMonitorTest, DistinctGroupsGetDistinctHistories) {
  DeviceMonitor monitor;

  monitor.addReading("group-1", temperatureAt(1, 20.0));
  monitor.addReading("group-2", temperatureAt(1, 21.0));

  EXPECT_NE(monitor.historyFor("group-1"), monitor.historyFor("group-2"));
  EXPECT_EQ(monitor.historyFor("group-1")->rowCount(), 1);
  EXPECT_EQ(monitor.historyFor("group-2")->rowCount(), 1);
}

TEST(DeviceMonitorTest, HistoryForIsStable) {
  DeviceMonitor monitor;
  HistoryModel *first = monitor.historyFor("group-1");
  EXPECT_EQ(first, monitor.historyFor("group-1"));
}

TEST(DeviceMonitorTest, ClearEmptiesEveryHistoryButKeepsIt) {
  DeviceMonitor monitor;
  monitor.addReading("group-1", temperatureAt(1, 20.0));
  HistoryModel *history = monitor.historyFor("group-1");

  monitor.clear();

  EXPECT_EQ(monitor.historyFor("group-1"), history);
  EXPECT_EQ(history->rowCount(), 0);
}
