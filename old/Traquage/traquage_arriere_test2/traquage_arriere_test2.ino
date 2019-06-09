#include <SPI.h>
#include <Wire.h>

#include <Pixy.h>
Pixy pixy;

#include <Servo.h>
Servo servo; // déclare un nom de Servomoteur


#define ADRESSE_PIXY2 13
#define ADRESSE_INTILLIGENCE_CENTRALE 8
#define ADRESSE_ROUE 19


int PositionX;


void setup()
{
  Serial.begin(9600);

  Wire.begin(ADRESSE_PIXY2);
  Serial.begin(9600);
  Serial.print("Starting...\n");
  pixy.init(); //démmare la caméra Pixy
}

void loop()
{
  uint16_t blocks;
  if (pixy.getBlocks())
  {
    blocks = pixy.getBlocks();
    //Serial.println(pixy.blocks[0].x);
    PositionX = pixy.blocks[0].x;
    int x = PositionX / 319 * 1024;

    Wire.beginTransmission(ADRESSE_ROUE);
    Wire.write(ADRESSE_PIXY2);
    Wire.write(x);
    Wire.endTransmission();
  }
}

