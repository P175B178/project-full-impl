#include "messages/StatusMessage.h"

#include "ProtocolExamples.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QTimeZone>

using namespace warden::messages;
using namespace warden::test;

namespace {

// The normal status example from protocol/; each test changes one field.
QJsonObject baseStatus() { return protocolExampleObject("status-normal.json"); }

QByteArray toPayload(const QJsonObject &object) { return QJsonDocument(object).toJson(QJsonDocument::Compact); }

} // namespace

TEST(StatusMessageTest, ParsesTheNormalExample) {
  const auto result = status_message::parse(protocolExample("status-normal.json"));

  ASSERT_TRUE(result.has_value()) << result.error().toStdString();
  EXPECT_EQ(result->deviceId, "group-1-pi5");
  EXPECT_EQ(result->state, status_message::State::Normal);

  const QDateTime expected(QDate(2026, 1, 2), QTime(3, 4, 5), QTimeZone::UTC);
  EXPECT_EQ(result->timestamp, expected);
  EXPECT_EQ(result->timestamp.timeSpec(), Qt::UTC);
}

TEST(StatusMessageTest, ParsesTheFaultExample) {
  const auto result = status_message::parse(protocolExample("status-fault.json"));

  ASSERT_TRUE(result.has_value()) << result.error().toStdString();
  EXPECT_EQ(result->state, status_message::State::Fault);
}

TEST(StatusMessageTest, ParsesTheOfflineExample) {
  const auto result = status_message::parse(protocolExample("status-offline.json"));

  ASSERT_TRUE(result.has_value()) << result.error().toStdString();
  EXPECT_EQ(result->state, status_message::State::Offline);
}

TEST(StatusMessageTest, ParsesWarningAndAlert) {
  QJsonObject object = baseStatus();
  object["state"]    = "warning";
  const auto warning = status_message::parse(toPayload(object));
  object["state"]    = "alert";
  const auto alert   = status_message::parse(toPayload(object));

  ASSERT_TRUE(warning.has_value()) << warning.error().toStdString();
  EXPECT_EQ(warning->state, status_message::State::Warning);
  ASSERT_TRUE(alert.has_value()) << alert.error().toStdString();
  EXPECT_EQ(alert->state, status_message::State::Alert);
}

TEST(StatusMessageTest, IgnoresUnknownFields) {
  QJsonObject object        = baseStatus();
  object["uptimeSeconds"]   = 3600;
  object["firmwareVersion"] = "1.2.0";

  const auto result = status_message::parse(toPayload(object));

  ASSERT_TRUE(result.has_value()) << result.error().toStdString();
  EXPECT_EQ(result->state, status_message::State::Normal);
}

TEST(StatusMessageTest, RejectsUnknownState) {
  QJsonObject object = baseStatus();
  object["state"]    = "degraded";

  EXPECT_FALSE(status_message::parse(toPayload(object)).has_value());
}

TEST(StatusMessageTest, RejectsMissingDeviceId) {
  QJsonObject object = baseStatus();
  object.remove("deviceId");

  EXPECT_FALSE(status_message::parse(toPayload(object)).has_value());
}

TEST(StatusMessageTest, RejectsMissingState) {
  QJsonObject object = baseStatus();
  object.remove("state");

  EXPECT_FALSE(status_message::parse(toPayload(object)).has_value());
}

TEST(StatusMessageTest, RejectsMissingTimestamp) {
  QJsonObject object = baseStatus();
  object.remove("timestamp");

  EXPECT_FALSE(status_message::parse(toPayload(object)).has_value());
}

TEST(StatusMessageTest, RejectsNonUtcTimestamp) {
  QJsonObject object  = baseStatus();
  object["timestamp"] = "2026-01-02T03:04:05+02:00";

  EXPECT_FALSE(status_message::parse(toPayload(object)).has_value());
}

TEST(StatusMessageTest, RejectsMalformedTimestamp) {
  QJsonObject object  = baseStatus();
  object["timestamp"] = "not-a-timestamp-Z";

  EXPECT_FALSE(status_message::parse(toPayload(object)).has_value());
}

TEST(StatusMessageTest, RejectsMalformedJson) {
  const auto result = status_message::parse(QByteArray("{ this is not json"));

  EXPECT_FALSE(result.has_value());
  EXPECT_FALSE(result.error().isEmpty());
}

TEST(StatusMessageTest, RejectsNonObjectJson) {
  EXPECT_FALSE(status_message::parse(QByteArray("\"offline\"")).has_value());
}

TEST(StatusMessageTest, NamesEveryState) {
  EXPECT_EQ(status_message::stateName(status_message::State::Normal), "Normal");
  EXPECT_EQ(status_message::stateName(status_message::State::Warning), "Warning");
  EXPECT_EQ(status_message::stateName(status_message::State::Alert), "Alert");
  EXPECT_EQ(status_message::stateName(status_message::State::Fault), "Fault");
  EXPECT_EQ(status_message::stateName(status_message::State::Offline), "Offline");
}
