// Code de l'Arduino Nano gérant le traquage avant du Minotaure
// Paul THONNEY et Maxime SCHARWATH
// 06.06.19

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
#define TEMPS_CONTINUE_RECHERCHE 2000
#define VITESSE_ROTATION_RECHERCHE 250
#define TEMPS_ATTENTE_RECHERCHE 250

long unsigned lastTimeViewObject = 0;
bool sensBalayage;
bool lastSideObject;
bool needToTrack = false;
int posObj; //position de l'objet

float posServo() {
  return mapfloat(servo.read(), 0, 180, -1, 1);
}
float posObject(int pos) {
  return mapfloat(pos, 0, 319, -1, 1);
}
void setup() {
  Serial.begin(9600);

  Wire.begin(ADRESSE_TRACKAGE);
  Wire.onReceive(receiveEvent);

  servo.attach(PIN_SERVO); //atribue une pin avec PWM au servomoteur
  servo.write(90); //met la tête droite

  Serial.print("Starting...\n");

  pixy.init(); //démarre la caméra Pixy
}

void loop() {
  tracking();
  //communication();
}

unsigned int count = 0;

void tracking() {
  uint16_t blocks; //nombre d'objets détecté par la Pixy
  if (pixy.getBlocks()) {
    blocks = pixy.getBlocks();
    lastTimeViewObject = millis();
    float posObj = posObject(pixy.blocks[0].x);
    int width = (pixy.blocks[0].width);
    int distance = map(width, 100, 15, 0, 255);
    if (distance < 0) {
      distance = 0;
    }
    if (distance > 255) {
      distance = 255;
    }
    Serial.println(distance);
    float posObjpositif = abs(posObj);
    int incrementation = mapfloat(posObjpositif, 0 , 1 , 0, 6);
    if (posObj < 0) {
      servo.write(servo.read() - incrementation);
      lastSideObject = 1;
    }
    else if (posObj > 0) {
      servo.write(servo.read() + incrementation);
      lastSideObject = 0;
    }
  }
  else {
    
    if (millis() < lastTimeViewObject + TEMPS_ATTENTE_RECHERCHE) {
      return;
    }
    if (!needToTrack) {
      servo.write(90);
      return;
    }
    count++;
    if (count % VITESSE_ROTATION_RECHERCHE != 0)return;

    if (millis() < lastTimeViewObject + TEMPS_CONTINUE_RECHERCHE) {
      sensBalayage = lastSideObject;
    }
    byte pos = servo.read();
    //Serial.println(String(sensBalayage) + " " + String(pos));
    if (sensBalayage) {
      servo.write(pos - 1);
    }
    else {
      servo.write(pos + 1);
    }

    if (pos <= 5 ) {
      sensBalayage = 0;
    }
    if (pos >= 175) {
      sensBalayage = 1;
    }
  }
}

void receiveEvent(int howMany) {
  Serial.println("howMany: " + String(howMany));
  int message = (uint8_t)Wire.read();
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}