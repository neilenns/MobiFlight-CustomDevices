This device forwardes the defined messages from mobiflight.genericI2C.device.json to I2C.
If you need more than 2 messages, extend the device.json file accordingly.

Connect an I2C device to the 2wire bus to receive the informations. This device must handle the messageID and the message.
The message is send as a string.


IMPORTANT!!
For now given an I2C address is not implemented in the connector.
Instead one pin can be defined. This one is interpreted as I2C address.
The connector will be adapted in the near future to also support I2C devices.
No changes are required for the firmware, except adapting this readme.