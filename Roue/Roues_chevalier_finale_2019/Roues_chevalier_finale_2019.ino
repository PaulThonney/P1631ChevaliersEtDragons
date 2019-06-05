#include <SoftPWM.h>
#include <Wire.h>

#define PWM_OUTPUT_MOTOR_L 4 // Pin 5 où sort le pwm du moteur 2
#define INPUT_4_MOTOR_L    3  // Pin 9 pour un sens
#define INPUT_3_MOTOR_L    9  // Pin 3 pour l'autre sens

#define PWM_OUTPUT_MOTOR_R 7 // Pin 6 où sort le pwm du moteur 1
#define INPUT_2_MOTOR_R    10  // Pin 10 pour un sens
#define INPUT_1_MOTOR_R    11  // Pin 11 pour l'autre sens

#define PIN_TEMP_R  A2
#define PIN_TEMP_L  A3
#define PIN_VENT_R  12
#define PIN_VENT_L  8
#define PIN_LED_URGENCE 2
#define EMERGENCY_TEMP 29
#define PWM_MIN 10
#define PWM_MAX 100

bool tempOverheating[] = {0, 0};

int stateMotors[][4] = {
  { -10, INPUT_1_MOTOR_R, INPUT_2_MOTOR_R, PWM_OUTPUT_MOTOR_R},
  {10, INPUT_3_MOTOR_L, INPUT_4_MOTOR_L, PWM_OUTPUT_MOTOR_L}
}; //value, ph1, ph2, pwm

void setup() {
  SoftPWMBegin();
  Serial.begin(9600);

  Wire.begin(19);
  Wire.onReceive(receiveEvent);

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
  digitalWrite(PIN_LED_URGENCE, LOW); //Eteint la led d'urgence
  pinMode (4, INPUT);//désactivation de pin
  pinMode (5, INPUT);//désactivation de pin
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
  stateMotors[id][0] = value;
  return true;
}

int getMotorValue(byte id) {
  return stateMotors[id][0];
}

void Motor(byte id) {
  int value = stateMotors[id][0];
  int pinPontH1 = stateMotors[id][1];
  int pinPontH2 = stateMotors[id][2];
  int pinPWM = stateMotors[id][3];

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
  SoftPWMSetPercent(pinPWM, abs(value));
}

void loopMotors() {
  Motor(0);
  Motor(1);
}

void loop() {
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
  if (howMany != 2) {
    Serial.println("ERREUR RECEPTION: " + String(howMany) + " bytes received");
    return;
  }
  byte motorId = Wire.read(); //premier byte
  byte motorValue = Wire.read(); //deuxieme byte //A voir je sais pas si on peut passer une valeur de -100 à 100; sinon on utilise 1 bit pour le signe et les 7 autres pour 0-100!!!
  if (!setMotorValue(motorId, motorValue)) {
    Serial.println("ERREUR MOTEUR");
  }
}
/////////////////////////////////////////////////////////////
