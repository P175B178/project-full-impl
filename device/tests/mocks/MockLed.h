#pragma once

#include "interfaces/Led.h"

#include <gmock/gmock.h>

namespace warden::test {

class MockLed : public Led {
public:
  MOCK_METHOD(void, show, (LedColor color, LedMode mode), (override));
  MOCK_METHOD(void, turnOff, (), (override));
};

} // namespace warden::test
