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
#define ADDR_TRACKING 0x10
#define WATCHTIMES_TIMEOUT 2500
#define PIN_SERVO 5
#define TEMPS_CONTINUE_RECHERCHE 2000
#define VITESSE_ROTATION_RECHERCHE 250
#define TEMPS_ATTENTE_RECHERCHE 250

long unsigned watchTimes = 0;
long unsigned lastTimeViewObject = 0;
bool isTracking = false;
bool sensBalayage;
bool lastSideObject;
bool needToTrack = false;
bool wantedMessage = false;
int posObj; //position de l'objet
byte distance;
byte angle;

int nbRequest;

float posServo() {
  return mapfloat(servo.read(), 0, 180, -1, 1);
}
float posObject(int pos) {
  return mapfloat(pos, 0, 319, -1, 1);
}
void setup() {
  Serial.begin(9600);

  Wire.begin(ADDR_TRACKING);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  servo.attach(PIN_SERVO); //atribue une pin avec PWM au servomoteur
  servo.write(90); //met la tête droite

  Serial.print("Starting...\n");

  pixy.init(); //démarre la caméra Pixy
  pixy.setLED(255, 0, 0); //set Pixy led RED
}

void loop() {
  if (millis() > watchTimes + WATCHTIMES_TIMEOUT) {
    //Serial.println("TIME OUT");
    needToTrack = false;
  }
  tracking();
}

unsigned int count = 0;


void requestEvent() {
  nbRequest++;
  Wire.write(byte(angle));
  Wire.write(byte(distance));
  Wire.write(byte(isTracking));

  Serial.println("REQUEST " + String(nbRequest));
  Serial.println(byte(angle));
  Serial.println(byte(distance));
  Serial.println(byte(isTracking));
}

void tracking() {
  uint16_t blocks; //nombre d'objets détecté par la Pixy
  isTracking = pixy.getBlocks() > 0;
  if (isTracking) {
    blocks = pixy.getBlocks();
    lastTimeViewObject = millis();
    float posObj = posObject(pixy.blocks[0].x);
    int width = (pixy.blocks[0].width);
    angle = servo.read();
    distance = map(width, 100, 15, 0, 255);
    if (distance < 0) {
      distance = 0;
    }
    if (distance > 255) {
      distance = 255;
    }
    //Serial.println(distance);
    float posObjpositif = abs(posObj);
    int incrementation = mapfloat(posObjpositif, 0 , 1 , 0, 6);
    if (posObj < 0) {
      servo.write(angle - incrementation);
      lastSideObject = 1;
    }
    else if (posObj > 0) {
      servo.write(angle + incrementation);
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
    //Serial.println(String(sensBalayage) + " " + String(angle));
    if (sensBalayage) {
      servo.write(angle - 1);
    }
    else {
      servo.write(angle + 1);
    }

    if (angle <= 5 ) {
      sensBalayage = 0;
    }
    if (angle >= 175) {
      sensBalayage = 1;
    }
  }
}

void receiveEvent(int howMany) {
  if (howMany == 0) {
    Serial.println("PING");
    watchTimes = millis();
    return;
  }
  int message = (uint8_t)Wire.read();
  Serial.println("Command: " + String(message));
  switch (message) {
    case 0x1E:
      needToTrack = true;
      break;
    case 0x2E:
      needToTrack = false;
      break;
  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
