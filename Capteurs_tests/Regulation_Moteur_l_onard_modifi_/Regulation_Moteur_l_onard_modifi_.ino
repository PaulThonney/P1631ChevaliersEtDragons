#include <Wire.h>
#include <SPI.h>


#define MOTORSPEEDPIN 5
#define SENSORPIN 2

unsigned long timerPID = 0;
unsigned long durationPID = 20;

unsigned long timerSensor = 0;
unsigned long durationSensor = 20;

unsigned long timerIntel = 0;
unsigned long durationIntel = 1000;

unsigned long timerI2c = 0;
unsigned long durationI2c = 500;

// Speed for the motor
byte motorCurrentSpeed = 0;

byte motorObjectiveSpeed = 0;

byte motorMeasuredSpeed = 0;
bool once = false;

int nbPulse;
bool flagPulse;
char buff[10];

byte nbSwitch = 255;
/*
   Variable for PID
*/
int error;
int previousError = 0;
int integral = 0;
int value = 0;
unsigned int pot = 0; // valeure du potentiomettre de test
/*
   Constant for the pid
*/
byte kP = 2;
byte kI = 2;
byte kD = 1;


byte prevNumber1 = 0;

void setup() {

  pinMode(MOTORSPEEDPIN, OUTPUT);

  Serial.begin(9600);
  Serial.println("Start");
  Wire.begin();
}

void loop() {
  pot = analogRead(0);
  motorObjectiveSpeed = map(pot, 0, 1023, 0, 60);
  if (timerSensor + durationSensor > millis()) {
    if (digitalRead(SENSORPIN) == HIGH && flagPulse == false) {
      flagPulse = true;
      nbPulse++;
    }
    if (digitalRead(SENSORPIN) == false && flagPulse == true) {
      flagPulse = false;
    }
  } else {
    if (millis() > timerIntel + durationIntel) {
      /* Serial.print("Objectif ");
        Serial.println(motorObjectiveSpeed);
        Serial.print("Pulse  ");
        Serial.println(nbPulse);
        Serial.print("error ");
        Serial.println(error);
        Serial.print("integral ");
        Serial.println(integral);
        Serial.print("value ");
        Serial.println(value);
        timerIntel = millis();//*/
    }

    timerSensor = millis();
    motorMeasuredSpeed = nbPulse;

    nbPulse = 0;
  }

  if (millis() > timerPID + durationPID) {
    //analogWrite(MOTORSPEEDPIN, regulationPID(motorObjectiveSpeed, motorMeasuredSpeed));
    regulationPID(motorObjectiveSpeed, motorMeasuredSpeed);
    timerPID = millis();
  }
}



/*
   Calculate the value to send to the motor depending of the speed wanted
*/
void regulationPID(byte objective, byte measuredValue) {
  error = objective - measuredValue;
  integral += (int)(error * 0.555);
  if (integral  > 255) {

    integral = 255;
  } else if (integral < -255) {
    integral = -255;
  }
  int derivative = 0;//(error - previousError) / durationPID;
  previousError = error;
  value = (error * kP * 0.5f + integral * kI + derivative * kD) * 4;
  if ( value > 255) {
    value = 255;
  } else if ((error * kP * 0.5f + integral * kI + derivative * kD) * 4 < -255) {
    value = -255;
  }
  if (value < 0) {
    motorCurrentSpeed = 0;

  } else {
    motorCurrentSpeed = value ;
  }
  if (motorCurrentSpeed == 255) {

  } else {

  }
  analogWrite(MOTORSPEEDPIN, motorCurrentSpeed);

}
