#include <SPI.h>
#include <Wire.h>

#include <Pixy.h>
Pixy pixy;

#include <Servo.h>
Servo servo; // déclare un nom de Servomoteur


#define ADRESSE_PIXY1 12
#define ADRESSE_Sensor1 14
#define ADRESSE_Sensor2 21
#define ADRESSE_INTILLIGENCE_CENTRALE 8
#define ADRESSE_LASER1 16 //avant
#define ADRESSE_LASER2 17 //arriière
#define ADRESSE_SERVO 20

#define Pin_servo 5

#define PIN_CAPTEUR_LUMINOSITE 9

int PositionX;
byte distance_avant[2];
byte distance_arriere[2];
int angle_old = 90;

unsigned long temps_actuel = 0;

void setup()
{
  Serial.begin(9600);

  pinMode(PIN_CAPTEUR_LUMINOSITE, INPUT);

  servo.attach(Pin_servo); //atribue une pin avec PWM au servomoteur
  servo.write(90); //met la tête droite

  Wire.begin(ADRESSE_PIXY1);
  Serial.begin(9600);
  Serial.print("Starting...\n");
  pixy.init(); //démmare la caméra Pixy
}

void loop()
{
  Serial.println(analogRead(PIN_CAPTEUR_LUMINOSITE));
  if (analogRead(PIN_CAPTEUR_LUMINOSITE) == 0)
  {
    if (servo.read() != 90)
      servo.write (90);
  }
  else
  {
    uint16_t blocks;
    if (pixy.getBlocks())
    {
      blocks = pixy.getBlocks();
      //Serial.println(pixy.blocks[0].x);
      PositionX = pixy.blocks[0].x;
      /*
        byte angle_new = (float)(pixy.blocks[0].x) / 319 * 75 + 52.5;
        if (angle_new != angle_old)
        {
        servo.write (angle_new);//donne un angle au servomoteur
        //Serial.println(angle_new);
        angle_old = angle_new;
        }
      */
      int incremenation = 0;
      if (PositionX > 170)
      {
        if (PositionX > 300)
          incremenation = 6;
        else if (PositionX > 260)
          incremenation = 5;
        else if (PositionX > 230)
          incremenation = 4;
        else if (PositionX > 200)
          incremenation = 3;
        else
          incremenation = 1;
        servo.write (servo.read() + incremenation);
      }
      else  if (PositionX < 150)
      {
        if (PositionX < 30)
          incremenation = 6;
        else if (PositionX < 60)
          incremenation = 5;
        else if (PositionX < 90)
          incremenation = 4;
        else if (PositionX < 120)
          incremenation = 3;
        else
          incremenation = 1;
        servo.write (servo.read() - incremenation);
      }
      //Serial.println(temps_actuel);
      // else
      //Serial.println("bingo");

      Serial.println(servo.read());
      Wire.beginTransmission(ADRESSE_INTILLIGENCE_CENTRALE);
      Wire.write(ADRESSE_SERVO);
      Wire.write(servo.read());
      Wire.endTransmission();

      temps_actuel = millis() / 1000;
    }
    else
    {
      //Serial.println(angle_old);
      if (millis() < 5000)
        servo.write (90);
      else if ( millis() / 1000 - temps_actuel > 2)
      {
        if (millis() % 10000 < 2000)
          servo.write (10);
        else if (millis() % 10000 < 4000)
          servo.write (60);
        else if (millis() % 10000 < 6000)
          servo.write (120);
        else if (millis() % 10000 < 8000)
          servo.write (170);
      }
    }
  }
}

