#pragma once

#include <Arduino.h>
#include "GenericI2C.h"

// only one entry required if you have only one custom device
enum {
    MOBIFLIGHT_GENERICI2C = 1
};
class MFCustomDevice
{
public:
    MFCustomDevice(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig);
    void detach();
    void update();
    void set(int8_t messageID, char *setPoint);

private:
    bool        getStringFromEEPROM(uint16_t addreeprom, char *buffer);
    bool        _initialized = false;
    GenericI2C *_myGenericI2C;
    uint8_t     _pin1, _pin2, _pin3;
    uint8_t     _customType = 0;
};
