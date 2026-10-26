#include "messages/CommandSerializer.h"

#include "ProtocolExamples.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace warden::messages;
using namespace warden::test;

namespace {

QJsonObject parse(const QByteArray &payload) { return QJsonDocument::fromJson(payload).object(); }

} // namespace

// Each command must be exactly the example in protocol/, the message the device's tests parse.

TEST(CommandSerializerTest, TemperatureThresholdMatchesTheExample) {
  EXPECT_EQ(parse(command_serializer::temperatureThreshold(28.0)),
            protocolExampleObject("config-temperature-threshold.json"));
}

TEST(CommandSerializerTest, HumidityThresholdMatchesTheExample) {
  EXPECT_EQ(parse(command_serializer::humidityThreshold(50.0)),
            protocolExampleObject("config-humidity-threshold.json"));
}

TEST(CommandSerializerTest, IntervalMatchesTheExample) {
  EXPECT_EQ(parse(command_serializer::interval(5)), protocolExampleObject("config-interval.json"));
}

TEST(CommandSerializerTest, ProducesCompactJson) {
  // Compact: no spaces or newlines.
  const QByteArray payload = command_serializer::interval(10);

  EXPECT_FALSE(payload.contains('\n'));
  EXPECT_FALSE(payload.contains(' '));
}
