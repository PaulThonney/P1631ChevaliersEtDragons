

#define SCL_PORT PORTD
#define SCL_PIN 7  // digital pin 7 emule SCL
#define SDA_PORT PORTD
#define SDA_PIN 6  // digital pin 6 SDA
#include <SoftI2CMaster.h> 

#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>

int nb_matrix;

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

int color;


void setup() {
  Serial.begin(115200);
  i2c_init();

  matrix.begin(0x70);
}
static const uint8_t PROGMEM
  smile_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10100101,
    B10011001,
    B01000010,
    B00111100 },
  neutral_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10111101,
    B10000001,
    B01000010,
    B00111100 },
  frown_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10011001,
    B10100101,
    B01000010,
    B00111100 };


  


void loop() {
 


  i2c_start((0x01<<1)|I2C_WRITE);
  i2c_write(1);
  i2c_stop();
  delay(2000);

  matrix.clear();
  matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_GREEN);
  matrix.writeDisplay();
  delay(500);
  
  i2c_start((0x01<<1)|I2C_WRITE);
  i2c_write(0);
  i2c_stop();
  delay(2000);

  matrix.clear();
  matrix.drawBitmap(0, 0, neutral_bmp, 8, 8, LED_YELLOW);
  matrix.writeDisplay();
  delay(500);

  
}
