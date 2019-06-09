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

#define Pin_servo 5


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
}

void loop()
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
    if (servo.read() != 90 && millis() / 1000 - temps_actuel > 1)
    {
      Serial.println("à 90°!!!");
      servo.write (90);
      //angle_old = 90;
    }
  }
  /*
    if (j % 50 == 0)
    {

      sprintf(buf, "Detected %d:\n", blocks);
      Serial.print(buf);
      for (k = 0; k < blocks; k++)
      {
        sprintf(buf, "  block %d: ", k);
        Serial.print(buf);
        //pixy.blocks[k].print();
        //Serial.println(pixy.blocks[k].x); //signatures
        //PositionX = pixy.blocks[0].x;
        //byte angle = (pixy.blocks[0].x) * (0.5625);
        //servo.write (angle);//donne un angle au servomoteur
        // Serial.println(angle);
        }

    }
  */
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



  /*
    if (blocks)
    {
      Wire.beginTransmission(ADRESSE_INTILLIGENCE_CENTRALE); // intilligence centrale
      Wire.write(ADRESSE_PIXY1);
      Wire.write(blocks);
      Wire.endTransmission();
    }
  */
}
