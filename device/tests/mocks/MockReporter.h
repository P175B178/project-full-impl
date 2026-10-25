#pragma once

#include "Reporter.h"

#include <gmock/gmock.h>

namespace warden::test {

class MockReporter : public Reporter {
public:
  MOCK_METHOD(void,
              reportReading,
              (const SmoothedReading &reading, std::chrono::system_clock::time_point time),
              (override));
  MOCK_METHOD(void, reportState, (State state, std::chrono::system_clock::time_point time), (override));
};

} // namespace warden::test
