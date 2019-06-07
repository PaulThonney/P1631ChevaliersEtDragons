/*************************************************** 
  This is a library for our SoftI2C LED Backpacks
  Written by Maxime Scharwath.
 ****************************************************/
#ifndef Adafruit_LEDBackpack_Soft_h
#define Adafruit_LEDBackpack_Soft_h

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif


#include "SoftwareI2C.h"

#include "Adafruit_GFX.h"

#define LED_ON 1
#define LED_OFF 0

#define LED_RED 1
#define LED_YELLOW 2
#define LED_GREEN 3
 


#define HT16K33_BLINK_CMD 0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_OFF 0
#define HT16K33_BLINK_2HZ  1
#define HT16K33_BLINK_1HZ  2
#define HT16K33_BLINK_HALFHZ  3

#define HT16K33_CMD_BRIGHTNESS 0xE0

#define SEVENSEG_DIGITS 5


// this is the raw HT16K33 controller
class Adafruit_LEDBackpack_Soft {
 public:
  Adafruit_LEDBackpack_Soft(byte SDA, byte SCL);
  void begin(uint8_t _addr);
  void setBrightness(uint8_t b);
  void blinkRate(uint8_t b);
  void writeDisplay(void);
  void clear(void);

  uint16_t displaybuffer[8]; 

  void init(uint8_t a);
 protected:
  uint8_t i2c_addr;
  SoftwareI2C sWire;
  byte SDA;
  byte SCL;
};

class Adafruit_AlphaNum4 : public Adafruit_LEDBackpack_Soft {
 public:
  Adafruit_AlphaNum4(byte SDA, byte SCL);

  void writeDigitRaw(uint8_t n, uint16_t bitmask);
  void writeDigitAscii(uint8_t n, uint8_t ascii, boolean dot = false);

 private:


};

class Adafruit_24bargraph : public Adafruit_LEDBackpack_Soft {
 public:
  Adafruit_24bargraph(byte SDA, byte SCL);

  void setBar(uint8_t bar, uint8_t color);

 private:
};


class Adafruit_8x16matrix : public Adafruit_LEDBackpack_Soft, public Adafruit_GFX {
 public:
  Adafruit_8x16matrix(byte SDA, byte SCL);

  void drawPixel(int16_t x, int16_t y, uint16_t color);

 private:
};

class Adafruit_8x16minimatrix : public Adafruit_LEDBackpack_Soft, public Adafruit_GFX {
 public:
  Adafruit_8x16minimatrix(byte SDA, byte SCL);

  void drawPixel(int16_t x, int16_t y, uint16_t color);

 private:
};

class Adafruit_8x8matrix : public Adafruit_LEDBackpack_Soft, public Adafruit_GFX {
 public:
  Adafruit_8x8matrix(byte SDA, byte SCL);

  void drawPixel(int16_t x, int16_t y, uint16_t color);

 private:
};

class Adafruit_BicolorMatrix : public Adafruit_LEDBackpack_Soft, public Adafruit_GFX {
 public:
  Adafruit_BicolorMatrix(byte SDA, byte SCL);

  void drawPixel(int16_t x, int16_t y, uint16_t color);

 private:
};


#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYTE 0

class Adafruit_7segment : public Adafruit_LEDBackpack_Soft {
 public:
  Adafruit_7segment(byte SDA, byte SCL);
  size_t write(uint8_t c);

  void print(char, int = BYTE);
  void print(unsigned char, int = BYTE);
  void print(int, int = DEC);
  void print(unsigned int, int = DEC);
  void print(long, int = DEC);
  void print(unsigned long, int = DEC);
  void print(double, int = 2);
  void println(char, int = BYTE);
  void println(unsigned char, int = BYTE);
  void println(int, int = DEC);
  void println(unsigned int, int = DEC);
  void println(long, int = DEC);
  void println(unsigned long, int = DEC);
  void println(double, int = 2);
  void println(void);
  
  void writeDigitRaw(uint8_t x, uint8_t bitmask);
  void writeDigitNum(uint8_t x, uint8_t num, boolean dot = false);
  void drawColon(boolean state);
  void printNumber(long, uint8_t = 2);
  void printFloat(double, uint8_t = 2, uint8_t = DEC);
  void printError(void);

  void writeColon(void);
    
 private:
  uint8_t position;
};
#endif // Adafruit_LEDBackpack_Soft_h

