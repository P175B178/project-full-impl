#pragma once

/// Abstract buzzer interface.
class Buzzer {
public:
  virtual void shortBeep(unsigned int count) = 0;
  virtual void longBeep(unsigned int count)  = 0;
  virtual void turnOff()                     = 0;
};
