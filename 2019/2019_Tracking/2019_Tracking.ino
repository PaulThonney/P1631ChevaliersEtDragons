/* Code de l'Arduino gérant le tracking avant avec la Pixy de devant
  @author: Paul THONNEY and Maxime SCHARWATH
  DATE: 06.06.19
*/
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

/*
   @func void loop Est la boucle centrale du code c'est elle qui appelle les fonctions principales
   @param null
   @return void
*/
void loop() {
  if (millis() > watchTimes + WATCHTIMES_TIMEOUT) {
    //Serial.println("TIME OUT");
    needToTrack = false;
  }
  tracking();
}

unsigned int count = 0;

/*
   @func void requestEvent envoie à l'intelligence centrale le fait qu'il se soit fait toucher et quel contact c'était
   @param null
   @return void
*/
void requestEvent() {
  nbRequest++;
  Wire.write(byte(angle));
  Wire.write(byte(distance));
  Wire.write(isTracking);

  Serial.println("REQUEST " + String(isTracking));
}

/*
   @func void tracking Tracke la cible à l'aide de la pixy et du servomoteur
   @param null
   @return void
*/
void tracking() {
  uint16_t blocks; //nombre d'objets détecté par la Pixy
  angle = servo.read();
  if (pixy.getBlocks() > 0) {
    isTracking=true;
    blocks = pixy.getBlocks();
    lastTimeViewObject = millis();
    float posObj = posObject(pixy.blocks[0].x);
    //Serial.println("Pixy x:"+String(posObj));
    int width = (pixy.blocks[0].width);
    distance = map(width, 100, 15, 0, 255);
    pixy.setLED(0, 0, 255-distance);
    if (distance < 0) {
      distance = 0;
    }
    if (distance > 255) {
      distance = 255;
    }
    //Serial.println(distance);
    float posObjpositif = abs(posObj);
    int incrementation = mapfloat(posObjpositif, 0 , 1 , 0, 6); // 0 à 6 sont les valeures qui marchaient le mieux
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
    isTracking=false;
    if (!needToTrack) {
      servo.write(90);
      return;
    }
    count++;
    if (count % VITESSE_ROTATION_RECHERCHE != 0)return;
    pixy.setLED(0, 255, 0);
    if (millis() < lastTimeViewObject + TEMPS_CONTINUE_RECHERCHE) {
      sensBalayage = lastSideObject;
    }
    //Serial.println("B: " + String(sensBalayage) + " A:" + String(angle));
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

/*
   @func void recieveEvent reçoit les info de l'intelligence centrale et oriente sur la bonne fonction dépendament du type de message
   @param int howMany
   @return void
*/
void receiveEvent(int howMany) {
  if (howMany == 0) {
    //Serial.println("PING");
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

/*
  @func float mapfloat Comme son nom l'indique c'est une fonction map mais qui comprends les floats
  @param float x 
  #param float in_min
  #param float in_max
  #param float out_min
  #param float out_max
  @return float la valeure mapée en float
*/
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
