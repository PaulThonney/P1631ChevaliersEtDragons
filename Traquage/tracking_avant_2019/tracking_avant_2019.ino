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
#define ADRESSE_INTELLIGENCE_CENTRALE 100
#define ADRESSE_TRACKAGE 20
#define PIN_SERVO 5
#define TEMPS_CONTINUE_RECHERCHE 2000
#define VITESSE_ROTATION_RECHERCHE 250
#define TEMPS_ATTENTE_RECHERCHE 250

long unsigned lastTimeViewObject = 0;
bool isTracking = false;
bool sensBalayage;
bool lastSideObject;
bool needToTrack = false;
bool wantedMessage = false;
int posObj; //position de l'objet
byte distance;

int nbRequest;

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
  Wire.onRequest(requestEvent);

  servo.attach(PIN_SERVO); //atribue une pin avec PWM au servomoteur
  servo.write(90); //met la tête droite

  Serial.print("Starting...\n");

  pixy.init(); //démarre la caméra Pixy
}

void loop() {
  tracking();
  communication();
}

unsigned int count = 0;


void requestEvent() {
  nbRequest++;
  Serial.println("REQUEST "+String(nbRequest));
  Serial.println(byte(servo.read()));
  Serial.println(byte(distance));
  Serial.println(byte(isTracking));

  Wire.write((servo.read()));
  Wire.write((distance));
  Wire.write((isTracking));
}

void communication() {
  return;
  if (!wantedMessage)return;
  wantedMessage = false;
  Serial.println("SEND DATA");
  Wire.beginTransmission(ADRESSE_INTELLIGENCE_CENTRALE);
  Wire.write(ADRESSE_TRACKAGE);
  Wire.endTransmission();
}

void tracking() {
  uint16_t blocks; //nombre d'objets détecté par la Pixy
  if (pixy.getBlocks()) {
    isTracking = true;
    blocks = pixy.getBlocks();
    lastTimeViewObject = millis();
    float posObj = posObject(pixy.blocks[0].x);
    int width = (pixy.blocks[0].width);
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
    isTracking = false;
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
  int message = (uint8_t)Wire.read();
  Serial.println("Command: " + String(message));
  switch (message) {
    case 0x1E:
      needToTrack = true;
      break;
    case 0x2E:
      needToTrack = false;
      break;
    case 0x3E:
      //wantedMessage = true;
      break;
  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
