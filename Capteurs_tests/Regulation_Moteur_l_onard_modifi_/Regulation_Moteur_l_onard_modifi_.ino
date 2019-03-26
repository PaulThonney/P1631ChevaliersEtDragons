#include <Wire.h>
#include <SPI.h>

#define MOTORDIRECTIONPIN 4
#define MOTORSPEEDPIN 5
#define ENCODERA 12
#define ENCODERB 11
#define SENSORPIN 2
#define LEDGREEN 10
#define LEDRED 9
#define SWITCH 8



int flagSensTrig = 0; // Sens of the encoder 0 when not specified 1 when clock wise 2 when counter clock wise
// flag for just one turn
bool flag = false;

unsigned long encoderTimer = 0;
unsigned long encoderDuration = 20;

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

byte motorObjectiveSpeed = 60;

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
/*
   Constant for the pid
*/
byte kP = 2;
byte kI = 2;
byte kD = 1;


byte prevNumber1 = 0;

void setup() {
  pinMode(MOTORDIRECTIONPIN, OUTPUT);
  pinMode(MOTORSPEEDPIN, OUTPUT);
  pinMode(ENCODERA, INPUT);
  pinMode(ENCODERB, INPUT);
  pinMode(LEDGREEN, OUTPUT);
  pinMode(LEDRED, OUTPUT);
  pinMode(SWITCH, INPUT);
  Serial.begin(9600);
  Serial.println("Start");
  digitalWrite(MOTORDIRECTIONPIN, 1); // Set motor to a rotation
  Wire.begin();
}

void loop() {
  motorObjectiveSpeed = analogRead(0) >> 4;
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
      Serial.print("Objectif ");
      Serial.println(motorObjectiveSpeed);
      Serial.print("Pulse  ");
      Serial.println(nbPulse);
      Serial.print("error ");
      Serial.println(error);
      Serial.print("integral ");
      Serial.println(integral);
      Serial.print("value ");
      Serial.println(value);
      timerIntel = millis();
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
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, LOW);
  } else {
    motorCurrentSpeed = value ;
  }
  if (motorCurrentSpeed == 255) {
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDRED, HIGH);
  } else {
    digitalWrite(LEDGREEN, HIGH);
    digitalWrite(LEDRED, LOW);
  }
  analogWrite(MOTORSPEEDPIN, motorCurrentSpeed);

}
