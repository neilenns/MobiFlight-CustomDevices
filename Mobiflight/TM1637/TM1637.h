#pragma once

#include "Arduino.h"
#include "LedControl_TM1637.h"

class TM1637
{
public:
    TM1637(uint8_t dio, uint8_t clk, uint8_t digits);
    void begin();
    void attach();
    void detach();
    void set(int8_t messageID, char *setPoint);
    void update();

private:
    bool          _initialised;
    uint8_t       _dio, _clk, _digits, _dpPosition;
    TM1637driver *_displayTM1637;
};