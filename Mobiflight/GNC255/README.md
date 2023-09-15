This custom device supports a 256x64 pixel OLED display with predifined outputs like the original GNC255.
The display uses hardware SPI for communication, so you MUST use the following pins:
Mega
* CLK: Pin 52
* Data: Pin 51
* CS: Pin 53 for the first display, for more choose as you want
* Pin 50 can NOT be used for any other device
Pico
* CLK: Pin 18
* Data: Pin 19
* CS: Pin 17 for the first display, for more choose as you want
* Pin 16 can NOT be used for any other device
For both boards D/C and Reset can be choosen as you want.

Connect your display accordingly the above used pins.
