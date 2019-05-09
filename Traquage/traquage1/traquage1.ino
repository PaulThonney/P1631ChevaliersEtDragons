#include <SPI.h>
#include <Wire.h>

#include <Pixy.h>
Pixy pixy;

#include <Servo.h>
Servo servo; // déclare un nom de Servomoteur

#include <VL53L0X.h>
VL53L0X sensor1; // Ceci permet de déclarer chaque capteur (on leur atribue n'importe quel nom).
VL53L0X sensor2; // la librairie enrgistre l'adresse de chaque capteur (0x29 par défaut)


#define ADRESSE_PIXY1 12
#define ADRESSE_Sensor1 14
#define ADRESSE_Sensor2 21
#define ADRESSE_INTILLIGENCE_CENTRALE 8
#define ADRESSE_LASER1 16 //avant
#define ADRESSE_LASER2 17 //arriière

#define Pin_servo 5

#define Reset1 8 //pins reset des capteur de distance
#define Reset2 9

int PositionX;
byte distance_avant[2];
byte distance_arriere[2];


void setup()
{
  Wire.begin(ADRESSE_PIXY1);
  Serial.begin(9600);
  Serial.print("Starting...\n");

  servo.attach(5); //atribue une pin avec PWM au servomoteur
  servo.write(90); //met la tête droite

  pixy.init(); //démmare la caméra Pixy

  pinMode(Reset1, OUTPUT);
  pinMode(Reset2, OUTPUT);
  digitalWrite(Reset1, LOW);
  digitalWrite(Reset2, LOW);
  delay(500);

  pinMode(Reset1, INPUT);
  delay(150);
  Serial.println("00");
  sensor1.init(true);
  Serial.println("01");
  delay(100);
  sensor1.setAddress((uint8_t)0x15);
  Serial.println("02");

  pinMode(Reset2, INPUT);
  delay(150);
  sensor2.init(true);
  Serial.println("03");
  delay(100);
  sensor2.setAddress((uint8_t)0x18);
  delay(100);
  Serial.println("04");

  sensor1.setTimeout(500);
  sensor2.setTimeout(500);
}
void loop()
{
  static int j = 0;
  int k;
  int i;
  char buf[32];
  uint16_t blocks;

  uint16_t laser_avant = sensor1.readRangeSingleMillimeters();
  uint16_t laser_arriere = sensor2.readRangeSingleMillimeters();

  distance_avant[0] = (laser_avant >> 8) & 0xFF; // "& 0xFF" permet de mettre les bits non utilisés à 0
  distance_avant[1] = (laser_avant & 0xFF);

  distance_arriere[0] = (laser_arriere >> 8) & 0xFF; // "& 0xFF" permet de mettre les bits non utilisés à 0
  distance_arriere[1] = (laser_arriere & 0xFF);

  //Serial.println(distance_arriere[0]);

  Serial.println(sensor1.readRangeSingleMillimeters());
  blocks = pixy.getBlocks();

  if (blocks)  {
    j++;
    PositionX = pixy.blocks[0].x;
    byte angle = (pixy.blocks[0].x) * (0.5625);
    //servo.write (angle);//donne un angle au servomoteur
    //Serial.println(angle);

    if (PositionX < 155 && PositionX > 165) {
      Serial.println(sensor1.readRangeSingleMillimeters());
    }


    /*if (j % 50 == 0)
      {*/

    sprintf(buf, "Detected %d:\n", blocks);
    Serial.print(buf);
    for (k = 0; k < blocks; k++)
    {
      //sprintf(buf, "  block %d: ", k);
      //Serial.print(buf);
      //pixy.blocks[k].print();
      //Serial.println(pixy.blocks[k].x); //signatures
      PositionX = pixy.blocks[0].x;
      byte angle = (pixy.blocks[0].x) * (0.5625);
      //servo.write (angle);//donne un angle au servomoteur
      // Serial.println(angle);
    }

    /*
      for (k = 0; k < blocks; k++)
      {
      Serial.print("Objet n°");
      Serial.println(k);
      if (pixy.blocks[k].signature == 1)
      {
        Serial.print("Rouge :");
      }
      else if (pixy.blocks[k].signature == 2)
      {
        Serial.print("Vert :");
      }
      else
      {
        Serial.print("Bleu :");
      }
      sprintf(buf, "largeur : %d \t longueur: %d \nx: %d\t\t y: %d", pixy.blocks[k].width, pixy.blocks[k].height, pixy.blocks[k].x, pixy.blocks[k].y);
      Serial.println(buf);
      }
    */
    //Serial.println(blocks);
  }

  /*
    if (blocks)
    {
      Wire.beginTransmission(ADRESSE_INTILLIGENCE_CENTRALE); // intilligence centrale
      Wire.write(ADRESSE_PIXY1);
      Wire.write(blocks);
      Wire.endTransmission();
    }
*/
    //Serial.println(sensor1.readRangeSingleMillimeters());
    Wire.beginTransmission(ADRESSE_INTILLIGENCE_CENTRALE); // intilligence centrale
    Wire.write(ADRESSE_LASER1);
    Wire.write(distance_avant, 2);
    Wire.endTransmission();
  /*
  Wire.beginTransmission(ADRESSE_INTILLIGENCE_CENTRALE); // intilligence centrale
  Wire.write(ADRESSE_LASER2);
  Wire.write(distance_arriere, 2);
  Wire.endTransmission();
*/
  delay (500);
}
