#include "TM1637.h"
#include "allocateMem.h"
#include "commandmessenger.h"

TM1637::TM1637(uint8_t dio, uint8_t clk, uint8_t digits)
{
    _dio    = dio;
    _clk    = clk;
    _digits = digits;

    if (!FitInMemory(sizeof(TM1637driver))) {
        // Error Message to Connector
        cmdMessenger.sendCmd(kStatus, F("Custom Device does not fit in Memory"));
        return;
    }
    _displayTM1637 = new (allocateMemory(sizeof(TM1637driver))) TM1637driver();
    _displayTM1637->begin(_dio, _clk, digits);
    _initialised = true;
}

void TM1637::begin()
{
}

void TM1637::attach()
{
}

void TM1637::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
}

void TM1637::set(int8_t messageID, char *setPoint)
{
    /* **********************************************************************************
        MessageID == -1 will be send from the connector when Mobiflight is closed
        Put in your code to shut down your custom device (e.g. clear a display)
        MessageID == -2 will be send from the connector when PowerSavingMode is entered
    ********************************************************************************** */
    uint8_t dpPosition = 0;

    switch (messageID) {
    case -1:
        _displayTM1637->clearDisplay();
    case -2:
        _displayTM1637->clearDisplay();
    case 0: // set digits
        _displayTM1637->showString(setPoint, 0, _dpPosition);
        break;
    case 1: // set decimal point, better would be to have it as a config parameter
        dpPosition = atoi(setPoint);
        if (dpPosition) {
            bitSet(_dpPosition, dpPosition);
        }
        _dpPosition = atoi(setPoint);
        break;
    case 2: // set brightness
        _displayTM1637->setIntensity(atoi(setPoint));
        break;
    default:
        break;
    }
}

void TM1637::update()
{
}
