#include <Wire.h>
#include <SPI.h>

#include <Pixy.h>
Pixy pixy;

#define ADRESSE_PIXY 13
#define ADRESSE_Sensor2 21
#define ADRESSE_INTILLIGENCE_CENTRALE 8

void setup()
{
  Wire.begin(ADRESSE_PIXY);
  Serial.begin(9600);
  Serial.print("Starting...\n");

  pixy.init(); //démmare la caméra Pixy

}
void loop()
{

  uint16_t blocks;

  blocks = pixy.getBlocks();

  if (blocks)
  {
    Wire.beginTransmission(ADRESSE_INTILLIGENCE_CENTRALE); // intilligence centrale
    Wire.write(ADRESSE_PIXY);
    Wire.write(blocks);
    Wire.endTransmission();
  }
}
