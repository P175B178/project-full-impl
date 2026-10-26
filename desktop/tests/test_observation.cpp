#include "messages/Observation.h"

#include "ProtocolExamples.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QTimeZone>

#include <optional>

using namespace warden::messages;
using namespace warden::test;

namespace {

// The temperature example from protocol/; each test changes one field.
QJsonObject baseTemperature() { return protocolExampleObject("observation-temperature.json"); }

QByteArray toPayload(const QJsonObject &object) { return QJsonDocument(object).toJson(QJsonDocument::Compact); }

} // namespace

TEST(ObservationTest, ParsesTheTemperatureExample) {
  const auto result = observation::parse(protocolExample("observation-temperature.json"));

  ASSERT_TRUE(result.has_value()) << result.error().toStdString();
  EXPECT_EQ(result->deviceId, "group-1-pi5");
  EXPECT_EQ(result->metric, observation::Metric::Temperature);
  EXPECT_DOUBLE_EQ(result->value, 23.4);
  EXPECT_EQ(result->unit, QString::fromUtf8("°C"));

  const QDateTime expected(QDate(2026, 1, 2), QTime(3, 4, 5), QTimeZone::UTC);
  EXPECT_EQ(result->effectiveTime, expected);
  EXPECT_EQ(result->effectiveTime.timeSpec(), Qt::UTC);
}

TEST(ObservationTest, ParsesTheHumidityExample) {
  const auto result = observation::parse(protocolExample("observation-humidity.json"));

  ASSERT_TRUE(result.has_value()) << result.error().toStdString();
  EXPECT_EQ(result->metric, observation::Metric::Humidity);
  EXPECT_DOUBLE_EQ(result->value, 45.0);
  EXPECT_EQ(result->unit, "%");
}

TEST(ObservationTest, IgnoresUnknownFields) {
  QJsonObject object        = baseTemperature();
  object["averagedSamples"] = 3;
  object["firmwareVersion"] = "1.2.0";
  object["location"]        = "lab-room-204";

  const auto result = observation::parse(toPayload(object));

  ASSERT_TRUE(result.has_value()) << result.error().toStdString();
  EXPECT_EQ(result->metric, observation::Metric::Temperature);
}

TEST(ObservationTest, RejectsWrongResourceType) {
  QJsonObject object     = baseTemperature();
  object["resourceType"] = "Patient";

  EXPECT_FALSE(observation::parse(toPayload(object)).has_value());
}

TEST(ObservationTest, RejectsMissingDeviceId) {
  QJsonObject object = baseTemperature();
  object.remove("deviceId");

  EXPECT_FALSE(observation::parse(toPayload(object)).has_value());
}

TEST(ObservationTest, RejectsMissingStatus) {
  QJsonObject object = baseTemperature();
  object.remove("status");

  EXPECT_FALSE(observation::parse(toPayload(object)).has_value());
}

TEST(ObservationTest, RejectsMissingValue) {
  QJsonObject object      = baseTemperature();
  object["valueQuantity"] = QJsonObject{{"unit", QString::fromUtf8("°C")}, {"code", "Cel"}};

  EXPECT_FALSE(observation::parse(toPayload(object)).has_value());
}

TEST(ObservationTest, RejectsNonNumericValue) {
  QJsonObject object      = baseTemperature();
  object["valueQuantity"] = QJsonObject{{"value", "warm"}, {"unit", QString::fromUtf8("°C")}};

  EXPECT_FALSE(observation::parse(toPayload(object)).has_value());
}

TEST(ObservationTest, RejectsUnknownLoincCode) {
  QJsonObject object = baseTemperature();
  object["code"]     = QJsonObject{{"system", "http://loinc.org"}, {"code", "0000-0"}};

  EXPECT_FALSE(observation::parse(toPayload(object)).has_value());
}

TEST(ObservationTest, RejectsNonUtcTimestamp) {
  QJsonObject object          = baseTemperature();
  object["effectiveDateTime"] = "2026-01-02T03:04:05+02:00";

  EXPECT_FALSE(observation::parse(toPayload(object)).has_value());
}

TEST(ObservationTest, RejectsTimestampWithoutZone) {
  QJsonObject object          = baseTemperature();
  object["effectiveDateTime"] = "2026-01-02T03:04:05";

  EXPECT_FALSE(observation::parse(toPayload(object)).has_value());
}

TEST(ObservationTest, RejectsMalformedTimestamp) {
  QJsonObject object          = baseTemperature();
  object["effectiveDateTime"] = "not-a-timestamp-Z";

  EXPECT_FALSE(observation::parse(toPayload(object)).has_value());
}

TEST(ObservationTest, RejectsMalformedJson) {
  const auto result = observation::parse(QByteArray("{ this is not json"));

  EXPECT_FALSE(result.has_value());
  EXPECT_FALSE(result.error().isEmpty());
}

TEST(ObservationTest, RejectsNonObjectJson) { EXPECT_FALSE(observation::parse(QByteArray("[1, 2, 3]")).has_value()); }

TEST(ObservationTest, FormatsAValueWithItsUnit) {
  EXPECT_EQ(observation::format(23.44, QString::fromUtf8("°C")), QString::fromUtf8("23.4 °C"));
  EXPECT_EQ(observation::format(45.0, "%"), "45.0 %");
  EXPECT_EQ(observation::format(std::nullopt, "%"), "—");
}
