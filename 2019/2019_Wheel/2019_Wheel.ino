// Maxime Scharwath
#include <SoftPWM.h>
#include <Wire.h>
#include <PID_v1.h>// PID by Brett Beauregard

#define ADDR_WHEEL 0x13

#define USE_PID true

#define WATCHTIMES_TIMEOUT 2500

#define NB_MOTORS 2
#define HOLE_NUMBER 20
#define WHEEL_RADIUS 30 // radius [mm]

#define PWM_OUTPUT_MOTOR_L 4 // Pin 5 où sort le pwm du moteur 2
#define INPUT_4_MOTOR_L    3  // Pin 9 pour un sens
#define INPUT_3_MOTOR_L    9  // Pin 3 pour l'autre sens
#define CAPTOR_MOTOR_L    A1

#define PWM_OUTPUT_MOTOR_R 7 // Pin 6 où sort le pwm du moteur 1
#define INPUT_2_MOTOR_R    10  // Pin 10 pour un sens
#define INPUT_1_MOTOR_R    11  // Pin 11 pour l'autre sens
#define CAPTOR_MOTOR_R    A0

#define PIN_TEMP_R  A2
#define PIN_TEMP_L  A3
#define PIN_VENT_R  12
#define PIN_VENT_L  8
#define PIN_LED_URGENCE 2
#define EMERGENCY_TEMP 35

#define KP 1
#define KI 0
#define KD 0

bool tempOverheating[2];

int pinMotors[NB_MOTORS][4] = {
  {INPUT_1_MOTOR_R, INPUT_2_MOTOR_R, PWM_OUTPUT_MOTOR_R, CAPTOR_MOTOR_R},
  {INPUT_3_MOTOR_L, INPUT_4_MOTOR_L, PWM_OUTPUT_MOTOR_L, CAPTOR_MOTOR_L}
}; //ph1, ph2, pwm, captor

double stateMotors[NB_MOTORS][4]; //Input,Output,Setpoint,Value

PID pid[NB_MOTORS] = {
  PID(&stateMotors[0][0], &stateMotors[0][1], &stateMotors[0][2], KP, KI, KD, DIRECT),
  PID(&stateMotors[1][0], &stateMotors[1][1], &stateMotors[1][2], KP, KI, KD, DIRECT)
};

bool previousSensorValue[NB_MOTORS];
unsigned int RPM[NB_MOTORS][3];//countHole, RPM, maxRPM
long prevtime = 0;

float getSpeed(int id) {
  return (WHEEL_RADIUS / 1000.0) * (2 * PI) * (RPM[id][1] / 60.0); // speed [m/s]
}

void(* resetFunc) (void) = 0; //RESET

unsigned long watchTimes;

void setup() {
  SoftPWMBegin();
  Serial.begin(9600);

  Wire.begin(ADDR_WHEEL);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  //Moteur droite
  SoftPWMSet(PWM_OUTPUT_MOTOR_R, 0);
  pinMode(INPUT_1_MOTOR_R, OUTPUT);
  pinMode(INPUT_2_MOTOR_R, OUTPUT);

  //Moteur gauche
  SoftPWMSet(PWM_OUTPUT_MOTOR_L, 0);
  pinMode(INPUT_3_MOTOR_L, OUTPUT);
  pinMode(INPUT_4_MOTOR_L, OUTPUT);

  digitalWrite(INPUT_1_MOTOR_R, LOW);
  digitalWrite(INPUT_2_MOTOR_R, LOW);
  digitalWrite(INPUT_3_MOTOR_L, LOW);
  digitalWrite(INPUT_4_MOTOR_L, LOW);


  pinMode(PIN_LED_URGENCE, OUTPUT);
  pinMode(PIN_VENT_R, OUTPUT);
  pinMode(PIN_VENT_L, OUTPUT);
  digitalWrite(PIN_LED_URGENCE, LOW); //Eteint la led d'urgence
  pinMode (4, INPUT);//désactivation de pin
  pinMode (5, INPUT);//désactivation de pin


  pinMode(CAPTOR_MOTOR_R, INPUT);
  pinMode(CAPTOR_MOTOR_L, INPUT);

  //PID
  for (int i = 0; i < NB_MOTORS; i++) {
    pid[i].SetOutputLimits(0, 100);
    pid[i].SetMode(AUTOMATIC);
  }
}

void checkRPM(int dur) {
  //Serial.println(String(digitalRead(CAPTOR_MOTOR_R)) + "," + String(digitalRead(CAPTOR_MOTOR_L)));
  for (int i = 0; i < NB_MOTORS; i++) {

    bool sensorValue = digitalRead(pinMotors[i][3]);
    if (sensorValue && !previousSensorValue[i]) {
      RPM[i][0]++;
    }
    previousSensorValue[i] = sensorValue;
  }

  long currtime = millis();
  if (currtime >= prevtime + dur) {
    long elapsedTime = currtime - prevtime;
    for (int i = 0; i < NB_MOTORS; i++) {
      RPM[i][1] = ( ( 60000 / elapsedTime ) * RPM[i][0]) / HOLE_NUMBER; //60000ms = 1min
      if (RPM[i][1] > RPM[i][2]) {
        RPM[i][2] = RPM[i][1];//MAX RPM
      }
      RPM[i][0] = 0;
      //Serial.println("RPM (" + String(i) + ") : " + String(RPM[i][1]) + ", " + String(getSpeed(i)) + " m/s");
      //Serial.println("RPM (" + String(i) + ") : " + String(RPM[i][0])+", "+String(RPM[i][1])+", "+String(RPM[i][2])+ ", " + String(getSpeed(i)) + " m/s");

stateMotors[i][0] = mapfloat(RPM[i][1], 0, 600, 0, 100); //Input RPM => 0-MAXRPM en 0-100
  pid[i].Compute();

    }
    prevtime = currtime;
  }
}


void checkTemp() {
  int tempR = getTemp(PIN_TEMP_R);
  if (tempR > EMERGENCY_TEMP) {
    tempOverheating[0] = true;
    digitalWrite(PIN_VENT_R, HIGH);
  } else {
    tempOverheating[0] = false;
    digitalWrite(PIN_VENT_R, LOW);
  }

  int tempL = getTemp(PIN_TEMP_L);
  if (tempL > EMERGENCY_TEMP) {
    tempOverheating[1] = true;
    digitalWrite(PIN_VENT_L, HIGH);
  } else {
    tempOverheating[1] = false;
    digitalWrite(PIN_VENT_L, LOW);
  }

  //GESTION DE LA LED
  if (tempOverheating[0] && tempOverheating[1]) {
    digitalBlink(PIN_LED_URGENCE, 125);
  } else if (tempOverheating[0] != tempOverheating[1]) {
    digitalBlink(PIN_LED_URGENCE, 250);
  } else {
    digitalWrite(PIN_LED_URGENCE, LOW);
  }


  //DEBUG
  if (false) {
    Serial.println("==TEMPERATURE==");
    Serial.println("Temp Right: " + String(tempR) + "°C");
    Serial.println("Temp Left: " + String(tempL) + "°C");
    Serial.println("===============");
  }
}

bool setMotorValue(byte id, int value) {
  if (id > (sizeof(stateMotors) / sizeof(stateMotors[0])))return false; //vérifie que l'id est valide
  if (value < -100 || value > 100)return false;//vérifie que la value est valide
  stateMotors[id][3] = value;
  stateMotors[id][2] = abs(value);//Setpoint
  return true;
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Motor(byte id) {
  double value = stateMotors[id][3];
  int pinPontH1 = pinMotors[id][0];
  int pinPontH2 = pinMotors[id][1];
  int pinPWM = pinMotors[id][2];
  int pinSensor = pinMotors[id][3];

  Serial.println("PID (" + String(RPM[id][1]) + ") : Input:" + String(stateMotors[id][0]) + ", Output:" + String(stateMotors[id][1]) + ", Setpoint:" + String(stateMotors[id][2]) + ", Value:" + String(stateMotors[id][3]) + ", ");


  if (false) {
    Serial.println("==MOTOR==");
    Serial.println("ID: " + String(id));
    Serial.println("value: " + String(value));
    Serial.println("pinPontH1: " + String(pinPontH1));
    Serial.println("pinPontH2: " + String(pinPontH2));
    Serial.println("pinPWM: " + String(pinPWM));
    Serial.println("===============");
  }

  if (value == 0) {
    digitalWrite(pinPontH1, LOW);
    digitalWrite(pinPontH2, LOW);
    analogWrite(pinPWM, 0);
    return;
  }

  digitalWrite(pinPontH1, (value < 0) ? LOW : HIGH);
  digitalWrite(pinPontH2, (value < 0) ? HIGH : LOW);

  if (USE_PID) {
    SoftPWMSetPercent(pinPWM, stateMotors[id][1]);//OUTPUT
  } else {
    SoftPWMSetPercent(pinPWM, stateMotors[id][2]);//SETPOINT
  }
}

void loopMotors() {
  checkRPM(250);//check rpm every 25ms
  Motor(0);
  Motor(1);
}

void loop() {
  if (millis() > watchTimes + WATCHTIMES_TIMEOUT) {
    Serial.println("TIME OUT");
    setMotorValue(0, 0);
    setMotorValue(1, 0);
  }
  checkTemp();
  loopMotors();
}

void digitalBlink(byte pin, unsigned int d) { // fait clignoter un pin avec un delais donné
  digitalWrite(pin, (millis() % (d * 2) > d));
}

int getTemp(byte pin) {
  return ( analogRead(pin) / 1024.0) * 5000 / 10;
}

////////////////////PARTIE I2C////////////////////
void receiveEvent(int howMany) {
  if (howMany == 0) {
    watchTimes = millis();
    Serial.println("PING");
    return;
  }

  if (howMany != 2) {
    Serial.println("ERREUR RECEPTION: " + String(howMany) + " bytes received");
    return;
  }
  byte motorId = Wire.read(); //premier byte
  byte motorValueByte = Wire.read(); //deuxieme byte

  int motorValue = motorValueByte;
  if (bitRead(motorValueByte, 7) == 1) {
    bitClear(motorValueByte, 7);
    motorValue = -motorValueByte;
  }

  if (!setMotorValue(motorId, motorValue)) {
    Serial.println("ERREUR MOTEUR");
  }
}

void requestEvent() {
  Serial.println("REQUEST");
  byte temp = 0;
  bitWrite(temp, 0, tempOverheating[0]);
  bitWrite(temp, 1, tempOverheating[1]);
  Wire.write(temp);
  Wire.write((byte *)&RPM[0][1], sizeof(int));
  Wire.write((byte *)&RPM[1][1], sizeof(int));
}
/////////////////////////////////////////////////////////////
