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
int angle_old = 90;

unsigned long temps_actuel = 0;


void setup()
{
  Serial.begin(9600);

  servo.attach(Pin_servo); //atribue une pin avec PWM au servomoteur
  servo.write(90); //met la tête droite

  Wire.begin(ADRESSE_PIXY1);
  Serial.begin(9600);
  Serial.print("Starting...\n");
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
  uint16_t laser_avant = sensor1.readRangeSingleMillimeters();
  uint16_t laser_arriere = sensor2.readRangeSingleMillimeters();

  distance_avant[0] = (laser_avant >> 8) & 0xFF; // "& 0xFF" permet de mettre les bits non utilisés à 0
  distance_avant[1] = (laser_avant & 0xFF);

  distance_arriere[0] = (laser_arriere >> 8) & 0xFF; // "& 0xFF" permet de mettre les bits non utilisés à 0
  distance_arriere[1] = (laser_arriere & 0xFF);

  Serial.println(sensor1.readRangeSingleMillimeters());

  uint16_t blocks;
  if (pixy.getBlocks())
  {
    blocks = pixy.getBlocks();
    //Serial.println(pixy.blocks[0].x);
    PositionX = pixy.blocks[0].x;

    if (PositionX > 159)
    {
      servo.write (servo.read() + 1);
    }
    else if (PositionX < 158)
      servo.write (servo.read() - 1);
    //Serial.println(temps_actuel);
    else
      Serial.println("bingo");
    temps_actuel = millis() / 1000;
  }
  else
  {
    //Serial.println(angle_old);
    if (servo.read() < 91 && servo.read() > 89  && millis() / 1000 - temps_actuel > 1)
    {
      //Serial.println("à 90°!!!");
      servo.write (90);
      //angle_old = 90;
    }
  }
}
