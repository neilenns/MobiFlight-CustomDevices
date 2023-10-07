#include "GenericI2C.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include <Wire.h>

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

GenericI2C::GenericI2C(uint8_t addrI2C)
{
    _addrI2C = addrI2C;
    _initialised = true;
}

void GenericI2C::begin()
{
    Wire.begin();
    Wire.setClock(400000);
}

void GenericI2C::attach()
{
}

void GenericI2C::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
}

void GenericI2C::set(int8_t messageID, char *setPoint)
{
    /* **********************************************************************************
        Each messageID has it's own value
        check for the messageID and define what to do.
        Important Remark!
        MessageID == -1 will be send from the connector when Mobiflight is closed
        Put in your code to shut down your custom device (e.g. clear a display)
        MessageID == -2 will be send from the connector when PowerSavingMode is entered
        Put in your code to enter this mode (e.g. clear a display)
    ********************************************************************************** */
    Wire.beginTransmission(_addrI2C);
    Wire.write(messageID);
    Wire.print(setPoint);
    Wire.endTransmission();
}

void GenericI2C::update()
{
    // Do something which is required regulary
}
