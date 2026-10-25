#pragma once

#include "MqttClient.h"

#include <gmock/gmock.h>

namespace warden::test {

class MockMqttClient : public mqtt::MqttClient {
public:
  MOCK_METHOD(void, publish, (const mqtt::MqttMessage &message), (override));
};

} // namespace warden::test
