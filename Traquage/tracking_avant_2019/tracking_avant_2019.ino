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

int PositionX; //position de l'objet

float posServo() {
  return mapfloat(servo.read(), 0, 180, -1, 1);
}
float posObject(int pos) {
  return mapfloat(pos, 0, 319, -1, 1);
}
void setup() {
  Serial.begin(9600);

  servo.attach(PIN_SERVO); //atribue une pin avec PWM au servomoteur
  servo.write(180); //met la tête droite
  Serial.print("Starting...\n");
  pixy.init(); //démarre la caméra Pixy
}

void loop() {
  uint16_t blocks; //nombre d'objets détecté par la Pixy
  if (pixy.getBlocks()) {
    blocks = pixy.getBlocks();
    PositionX = pixy.blocks[0].x;
    //Serial.println(posObject(pixy.blocks[0].x));
    float posObj = posObject(pixy.blocks[0].x);
    float diff = 5 * abs(abs(posObj) - abs(posServo()));
    Serial.println(diff);
    if (posObj < 0) {
      servo.write(servo.read() - diff);
    }
    else if (posObj > 0) {
      servo.write(servo.read() + diff);
    }


  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
