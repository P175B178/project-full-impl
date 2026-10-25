#include "MqttPublisher.h"

#include "mocks/MockMqttClient.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <chrono>

using namespace warden;
using namespace warden::mqtt;
using namespace warden::test;
using ::testing::AllOf;
using ::testing::Field;
using ::testing::InSequence;
using ::testing::ResultOf;
using ::testing::StrictMock;

namespace {
/// The deviceId and value a JSON payload carries.
auto carries(const std::string &deviceId, double value) {
  return Field(&MqttMessage::payload, ResultOf(
                                          [](const std::string &payload) {
                                            const auto json = nlohmann::json::parse(payload);
                                            return std::pair{json["deviceId"].get<std::string>(),
                                                             json["valueQuantity"]["value"].get<double>()};
                                          },
                                          std::pair{deviceId, value}));
}
} // namespace

TEST(MqttPublisherTest, PublishesReadingOnTheGroupsSensorTopics) {
  StrictMock<MockMqttClient> client;
  MqttPublisher publisher{client, "group-7", "group-7-pi5"};
  {
    const InSequence inOrder;
    EXPECT_CALL(client, publish(AllOf(Field(&MqttMessage::topic, "sensors/group-7/temperature"),
                                      carries("group-7-pi5", 21.5))));
    EXPECT_CALL(client,
                publish(AllOf(Field(&MqttMessage::topic, "sensors/group-7/humidity"), carries("group-7-pi5", 40.0))));
  }
  publisher.reportReading({.temperature = 21.5F, .humidity = 40.0F}, std::chrono::system_clock::now());
}

TEST(MqttPublisherTest, ReadingsAreSentOnceAndNotRetained) {
  StrictMock<MockMqttClient> client;
  MqttPublisher publisher{client, "group-1", "group-1-pi5"};
  EXPECT_CALL(client, publish(AllOf(Field(&MqttMessage::qos, 0), Field(&MqttMessage::retained, false)))).Times(2);
  publisher.reportReading({.temperature = 21.5F, .humidity = 40.0F}, std::chrono::system_clock::now());
}

TEST(MqttPublisherTest, PublishesStatusRetainedUntilConfirmed) {
  StrictMock<MockMqttClient> client;
  MqttPublisher publisher{client, "group-7", "group-7-pi5"};
  EXPECT_CALL(client, publish(AllOf(Field(&MqttMessage::topic, "sensors/group-7/status"), Field(&MqttMessage::qos, 1),
                                    Field(&MqttMessage::retained, true))));
  publisher.reportState(State::Alert, std::chrono::system_clock::now());
}
