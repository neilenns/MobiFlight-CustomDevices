This is the support for TM1637 7 segment displays.
Most work has been done from GiorgioCC to get these displays to run.
Modifications where done to support only TM1637 displays and not additional MAX7219 displays
as they are a basic device.
Once the TM1637 support gets implemented as a standard device this custom devive gets deleted.

Choose in the connector when you add a custom device if you have a 4 digit or 6 digit display.
Define an output in the connector, choose the message type "Set decimal point" and fill in
the digit number under "Value" on which digit a decimal point should be displayed.
This might be changed in the future.
