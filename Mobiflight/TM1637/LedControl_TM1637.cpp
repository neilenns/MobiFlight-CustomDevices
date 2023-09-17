// =======================================================================
// @file        LedControl_TM1637.cpp
//
// @project     MobiFlight custom Firmware
//
// @author      GiorgioCC (g.crocic@gmail.com) - 2023-06-29
// @modifiedby  GiorgioCC - 2023-07-09 20:13
//
// A library for controlling LED 7-segment displays with either
// a MAX7219/MAX7221 or a TM1637 (4/6 digit) driver
// Portions of code derived from:
// - TM1637TinyDisplay - TM1637 Tiny Display library by Jason A. Cox
//   (https://github.com/jasonacox)
//
// Further modifications from elral -2023-09
// Only TM1637 4/6 digits are supported, all aprts for the MAX7219 are deleted
// The intend is to have a first custom device version until the TM1637
// support gets merged as basic device
//
// =======================================================================

#include "LedControl_TM1637.h"

// =======================================================================
// Common Definitions
// =======================================================================

// Segments to be switched on for characters and digits on 7-Segment Displays
// bit/segment sequence: dABCDEFG
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---   .d
//      D

const static byte charTable[128] PROGMEM = {
    // 0         ,1         ,2         ,3         ,4         ,5         ,6         ,7
    0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011, 0b01011111, 0b01110000,
    // 8         ,9         ,A         ,B         ,C         ,D         ,E         ,F
    0b01111111, 0b01111011, 0b01110111, 0b00011111, 0b00001101, 0b00111101, 0b01001111, 0b01000111,
    // <0x10>    ,<0x11>    ,<0x12>    ,<0x13>    ,<0x14>    ,<0x15>    ,<0x16>    ,<0x17>
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // <0x18>    ,<0x19>    ,<0x1A>    ,<0x1B>    ,<0x1C>    ,<0x1D>    ,<0x1E>    ,<0x1F>
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // (space)    ,!          ,"          ,#          ,$          ,%          ,&          ,'
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    // (         ,)         ,*         ,+         ,,         ,-         ,.         ,/
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10000000, 0b00000001, 0b10000000, 0b00100101,
    // 0         ,1         ,2         ,3         ,4         ,5         ,6         ,7
    0b01111110, 0b00110000, 0b01101101, 0b01111001, 0b00110011, 0b01011011, 0b01011111, 0b01110000,
    // 8         ,9         ,:         ,;         ,<         ,=         ,>         ,?
    0b01111111, 0b01111011, 0b00000000, 0b00000000, 0b00000000, 0b00001001, 0b00000000, 0b00000000,
    // @         ,A         ,0b         ,C         ,D         ,E         ,F         ,G
    0b00000000, 0b01110111, 0b01111111, 0b01001110, 0b01111110, 0b01001111, 0b01000111, 0b01011111,
    // H         ,I         ,J         ,K         ,L         ,M         ,N         ,O
    0b00110111, 0b00110000, 0b01111100, 0b00000000, 0b00001110, 0b00000000, 0b01110110, 0b01111110,
    // P         ,Q         ,R         ,S         ,T         ,U         ,V         ,W
    0b01100111, 0b01111110, 0b01110111, 0b01011011, 0b01000110, 0b00111110, 0b00000000, 0b00000000,
    // X         ,Y         ,Z         ,[         ,\         ,]         ,^         ,_
    0b00000000, 0b00110011, 0b01101101, 0b01001110, 0b00010011, 0b01111000, 0b00000000, 0b00001000,
    // `         ,a         ,0b         ,c         ,d         ,e         ,f         ,g
    0b00000000, 0b01111101, 0b00011111, 0b00001101, 0b00111101, 0b01001111, 0b01000111, 0b01111011,
    // h         ,i         ,j         ,k         ,l         ,m         ,n         ,o
    0b00110111, 0b00110000, 0b00111000, 0b00000000, 0b00000110, 0b00000000, 0b00010101, 0b00011101,
    // p         ,q         ,r         ,s         ,t         ,u         ,v         ,w
    0b01100111, 0b01110011, 0b00000101, 0b01011011, 0b00001111, 0b00011100, 0b00000000, 0b00000000,
    // x         ,y         ,z         ,{         ,|         ,}         ,~         ,(delete)
    0b00000000, 0b00100111, 0b01101101, 0b01001110, 0b00000000, 0b01111000, 0b00000000, 0b00000000};

// =======================================================================
// TM1637 Definitions
// =======================================================================

// TM1637 Commands
//
// Communication Sequence (Automatic Address)
//     Cmd1: Start + CmdSetData + ACK + Stop +
//     Cmd2+Data: Start + CmdSetAddress + ACK + (Data + ACK) * N +
//     Cmd3: Start + CmdDisplay + ACK + Stop
//
// Communication Sequence (Fixed Address)
//     Cmd1: Start + CmdSetData + ACK + Stop
//     Cmd2+Data: (Start + Command + ACK + Data + ACK) * N + Stop +
//     Cmd3: Start + Command3 + ACK + Stop
//
// CmdSetData - Data command settings (byte) - TM1637_I2C_COMM1
// B7 B6 B5 B4 B3 B2 B1 B0 - Function Description
// 0  1  0  0  _  _  0  0  - (Data read/write) Write data to display register
// 0  1  0  0  _  _  1  0  - (Data read/write) Read key scan data
// 0  1  0  0  _  0  _  _  - (Address mode) Automatic address adding
// 0  1  0  0  _  1  _  _  - (Address mode) Fixed address
// 0  1  0  0  0  _  _  _  - (Test mode) Normal mode
// 0  1  0  0  1  _  _  _  - (Test mode) Test mode
//
// CmdSetAddress - Set Address - Digit (byte) - TM1637_I2C_COMM2
// B7 B6 B5 B4 B3 B2 B1 B0 - Function Description
// 1  1  0  0  0  0  0  0  - Digit 1 - C0H Grid1
// 1  1  0  0  0  0  0  1  - Digit 2 - C1H Grid2
// 1  1  0  0  0  0  1  0  - Digit 3 - C2H Grid3
// 1  1  0  0  0  0  1  1  - Digit 4 - C3H Grid4
// 1  1  0  0  0  1  0  0  - Digit 5 - C4H Grid5
// 1  1  0  0  0  1  0  1  - Digit 6 - C5H Grid6
//
// CmdDisplay - Set Display - Digit (byte) - TM1637_I2C_COMM3
// B7 B6 B5 B4 B3 B2 B1 B0 - Function Description
// 1  0  0  0  _  0  0  0  - Brightness - Pulse width is set as 1/16
// 1  0  0  0  _  0  0  1  - Brightness - Pulse width is set as 2/16
// 1  0  0  0  _  0  1  0  - Brightness - Pulse width is set as 4/16
// 1  0  0  0  _  0  1  1  - Brightness - Pulse width is set as 10/16
// 1  0  0  0  _  1  0  0  - Brightness - Pulse width is set as 11/16
// 1  0  0  0  _  1  0  1  - Brightness - Pulse width is set as 12/16
// 1  0  0  0  _  1  1  0  - Brightness - Pulse width is set as 13/16
// 1  0  0  0  _  1  1  1  - Brightness - Pulse width is set as 14/16
// 1  0  0  0  0  _  _  _  - Display OFF
// 1  0  0  0  1  _  _  _  - Display ON

#define TM1637_I2C_COMM1  0x40 // CmdSetData       0b01000000
#define TM1637_I2C_COMM2  0xC0 // CmdSetAddress    0b11000000
#define TM1637_I2C_COMM3  0x80 // CmdDisplay       0b10000000
#define TM1637_I2C_COMM1F 0x44 // CmdSetData - fixedAddress    0b11000100
// =======================================================================

// Digit sequence map for 6 digit displays
const uint8_t digitmap[] = {2, 1, 0, 5, 4, 3};

// Configuration:
// csPin < 0xFD -> MAX72xx
// csPin = 0xFD -> TM1637 4-digit
// csPin = 0xFE -> TM1637 6-digit
// csPin = 0xFF -> uninitialized

void TM1637driver::begin(uint8_t dataPin, uint8_t clkPin, uint8_t numDigits)
{
    IO_DTA = dataPin;
    IO_CLK = clkPin;

    maxUnits = numDigits;
    // Both pins are set as inputs, allowing the pull-up resistors to pull them up
    pinMode(IO_CLK, INPUT_PULLUP);
    pinMode(IO_DTA, INPUT_PULLUP);
    digitalWrite(IO_CLK, LOW); // Prepare '0' value as dominant
    digitalWrite(IO_DTA, LOW); // Prepare '0' value as dominant
    clearDisplay();
    // setIntensity(0, MAX_BRIGHTNESS);
    brightness = MAX_BRIGHTNESS;
    shutdown(true);
}

void TM1637driver::shutdown(bool b)
{
    uint8_t bri = brightness >> 1;
    if (!b) bri |= 0x08;
    // Write COMM3 + intensity
    start();
    writeByte(TM1637_I2C_COMM3 + bri);
    stop();
}

void TM1637driver::setIntensity(uint8_t intensity)
{
    if (intensity > 15) intensity = 15;
    brightness = intensity;

    if (intensity > 0) {
        if (intensity > 1) intensity >>= 1;
        intensity |= 0x08;
    }
    // Write COMM3 + intensity
    start();
    writeByte(TM1637_I2C_COMM3 + intensity);
    stop();
}

void TM1637driver::clearDisplay()
{
    memset(rawdata, 0, maxUnits);
    writeBuffer();
}

void TM1637driver::setDigit(uint8_t digit, uint8_t value, bool dp, bool sendNow)
{
    if ((value > 15) && (value != '-')) value = (uint8_t)' '; // Use space for invalid digit
    if (dp) value |= 0x80;
    setPattern(digit, value, sendNow);
}

void TM1637driver::setChar(uint8_t digit, char value, bool dp, bool sendNow)
{
    uint8_t v;
    v = (uint8_t)value;  // Get rid of signedness
    if (v > 127) v = 32; // undefined: replace with space char
    if (dp) v |= 0x80;
    setPattern(digit, v, sendNow);
}

void TM1637driver::setPattern(uint8_t digit, uint8_t value, bool sendNow)
{
    if (digit > maxUnits - 1) return;
    uint8_t v;
    v = pgm_read_byte_near(charTable + (value & 0x7F));
    // Original data for MAX has the bit sequence: dABCDEFG
    // Common TM1637 boards are connected so that they require: dGFEDCBA
    // For the least effort, we alter the byte to be transmitted as: ABCDEFGd
    // and then just transmit them reversed (from LSb to MSb)
    v <<= 1;
    if (value & 0x80) v |= 0x01;
    rawdata[(maxUnits - 1) - digit] = v; // Change only the individual affected digit in static buffer
    if (sendNow) {
        writeDigits(digit, 1);
    }
}

// ------------------------------------------------
// TM-specific driver methods
// ------------------------------------------------

void TM1637driver::start()
{
    pinMode(IO_DTA, OUTPUT);
    bitDelay();
}

void TM1637driver::stop()
{
    pinMode(IO_DTA, OUTPUT);
    bitDelay();
    pinMode(IO_CLK, INPUT);
    bitDelay();
    pinMode(IO_DTA, INPUT);
    bitDelay();
}

bool TM1637driver::writeByte(uint8_t data, bool rvs)
{
    uint8_t msk = (rvs ? 0x80 : 0x01);
    for (uint8_t i = 0; i < 8; i++) {
        // CLK low
        pinMode(IO_CLK, OUTPUT);
        bitDelay();
        // Set data bit
        pinMode(IO_DTA, (data & msk) ? INPUT : OUTPUT);
        bitDelay();
        // CLK high
        pinMode(IO_CLK, INPUT);
        bitDelay();
        data = (rvs ? data << 1 : data >> 1);
    }
    // Wait for acknowledge
    // CLK to zero
    pinMode(IO_CLK, OUTPUT);
    pinMode(IO_DTA, INPUT);
    bitDelay();
    // CLK to high
    pinMode(IO_CLK, INPUT);
    bitDelay();
    uint8_t ack = digitalRead(IO_DTA);
    if (ack == 0) pinMode(IO_DTA, OUTPUT);
    bitDelay();
    pinMode(IO_CLK, OUTPUT);
    bitDelay();
    return ack;
}

// =========================================================
//   Methods for extended library
// =========================================================

void TM1637driver::writeDigits(uint8_t startd, uint8_t len)
{
    bool    is4Digit = (maxUnits == 4);
    uint8_t b;

    // Write COMM1
    start();
    writeByte(TM1637_I2C_COMM1);
    stop();

    uint8_t pos = (maxUnits - 1) - startd;
    b           = (is4Digit ? pos : digitmap[pos + len - 1]);

    start();
    writeByte(TM1637_I2C_COMM2 + b);
    // Write the data bytes
    if (pos + len > maxUnits) len = maxUnits - pos;
    uint8_t k;
    for (b = 0; b < len; b++) {
        k = (is4Digit ? b : len - b - 1);
        writeByte(rawdata[pos + k], true);
    }
    stop();
}

void TM1637driver::showNumber(int32_t num, bool isHex, uint8_t dots, bool leading_zero, uint8_t roffset)
{
    uint8_t digits[8];
    uint8_t pos;
    bool    minusRequired = (num < 0);
    if (minusRequired) num = -num;

    for (uint8_t i = 0; i < 8; i++)
        digits[i] = (uint8_t)' ';

    pos = (maxUnits - 1) - roffset; // Reverse to use as counter
    if (num == 0) {
        if (leading_zero) {
            for (uint8_t i = 0; i < pos; i++) {
                digits[i] = '0';
            }
        }
        digits[pos] = '0';
    } else {
        uint8_t dval;
        do {
            if (num == 0) {
                if (leading_zero) {
                    digits[pos] = '0';
                    if (minusRequired && pos == 0) digits[0] = '-';
                } else {
                    if (minusRequired) digits[pos] = '-';
                    pos = 0;
                }
            } else {
                if (!isHex) {
                    dval        = num % 10;
                    digits[pos] = '0' + dval;
                } else {
                    dval        = num & 0x0F;
                    digits[pos] = ((dval > 9) ? 'A' - 10 : '0') + dval;
                }
            }
            if (!isHex) {
                num /= 10;
            } else {
                num >>= 4;
            }
        } while ((pos--) > 0);
    }
    showString((char *)digits, 0, dots);
}

void TM1637driver::showString(char *s, uint8_t loffset, uint8_t dots)
{
    uint8_t msk = 0x80 >> loffset;
    for (uint8_t d = loffset; d < maxUnits && (*s != 0); d++) {
        uint8_t pos = (maxUnits - 1) - d;
        setChar(pos, *s++, ((dots & msk) != 0), false);
        msk >>= 1;
    }
    writeBuffer();
}

// end
