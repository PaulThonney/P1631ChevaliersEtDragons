// Code de l'Arduino Nano gérant le traquage avant du Minotaure
// Dany VALADO
// 01.07.2018
// Ce qu'il reste à ajouter dans le code :
//  - mode pause
//  - les capteurs laser (Lidar)
//  - fluidifier la rotation du servo lors de la recherche de cible
//  - contrôler que les proportions de la taille de l'objet détecté par la Pixy corresponde à la taille de référence
//  - développer le code pour qu'il puisse prendre en compte plusieurs cibles

#include <SPI.h> //comunication avec la Pixy
#include <Wire.h> //I2C

#include <Pixy.h>
Pixy pixy; //donne un nom à la pixy

#include <Servo.h>
Servo servo; // donne un nom de Servomoteur

//adresse I2C
#define ADRESSE_PIXY1 12
#define ADRESSE_INTELLIGENCE_CENTRALE 1
#define ADRESSE_TRACKAGE 20

#define PIN_SERVO 5

int PositionX; //position de l'objet
byte distance_avant[2]; // distance à envoyer par I2C
byte distance_arriere[2];

byte lastAngle;

unsigned long temps = 0; //temps du dernier moment où il aperçoit sa cible

void setup()
{
  Serial.begin(9600);

  servo.attach(PIN_SERVO); //atribue une pin avec PWM au servomoteur
  servo.write(90); //met la tête droite

  Wire.begin(ADRESSE_PIXY1);
  Serial.print("Starting...\n");
  pixy.init(); //démarre la caméra Pixy
}

void loop()
{
  uint16_t blocks; //nombre d'objets détecté par la Pixy
  if (pixy.getBlocks()){
    blocks = pixy.getBlocks();
    Serial.println(pixy.blocks[0].x);
    PositionX = pixy.blocks[0].x;

    int incremenation = 0; //angle ajouté ou soustré au servo pour suivre sa cible

    if (PositionX > 170)
    {
      if (PositionX > 300) // plus sa cible est loin du centre, plus vite il doit tourner la tête pour ne pas la perdre
        incremenation = 6;
      else if (PositionX > 260) // si la tête ne tourne pas assez vite il perd sa cible
        incremenation = 5;
      else if (PositionX > 230) //
        incremenation = 4;
      else if (PositionX > 200)
        incremenation = 3;
      else
        incremenation = 1;
      servo.write (servo.read() + incremenation); // tourne la tête à droite
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
      servo.write (servo.read() - incremenation); //tourne la tête à gauche
    }

    //Serial.println(servo.read());
    byte angle = servo.read();
    if (abs(lastAngle - angle) > 5) {
      lastAngle = angle;
      Wire.beginTransmission(ADRESSE_INTELLIGENCE_CENTRALE);
      Wire.write(ADRESSE_TRACKAGE); //permet à l'INTELLIGENCE centrale de savoir qui lui parle
      Wire.write(0); // ID de la pixy
      Wire.write(angle); // donne l'angle de la direction à prendre pour se déplacer
      Wire.endTransmission();
    }
    //Serial.println(servo.read());
    temps = millis() / 1000; //temps divisé par 1000 pour remplir la mémoire de la variable moins rapidement

  }
  else{
    if (millis() < 5000) // 5s au départ pour fixer la tête
      servo.write (90);
    else if ( millis() / 1000 - temps > 2) // s'il perd sa cible depuis plus de 2s
    {
      if (millis() % 10000 < 2000) // tourne la tête toutes les 2s
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
