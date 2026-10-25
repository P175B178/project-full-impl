#pragma once

#include "interfaces/Buzzer.h"

#include <gmock/gmock.h>

namespace warden::test {

class MockBuzzer : public Buzzer {
public:
  MOCK_METHOD(void, shortBeep, (unsigned int count), (override));
  MOCK_METHOD(void, longBeep, (unsigned int count), (override));
  MOCK_METHOD(void, turnOff, (), (override));
};

} // namespace warden::test
