#include <SoftPWM.h>
#include <Wire.h>

#define ADDR_WHEEL 0x13

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

int stateMotors[][6] = {
  {0, 0, INPUT_1_MOTOR_R, INPUT_2_MOTOR_R, PWM_OUTPUT_MOTOR_R},
  {0, 0, INPUT_3_MOTOR_L, INPUT_4_MOTOR_L, PWM_OUTPUT_MOTOR_L}
}; //value, measured, ph1, ph2, pwm, captor

unsigned long timerMotors[2][2];
int nbPulseMotors[2];
bool flagPulseMotors[2];

void setup() {
  SoftPWMBegin();
  Serial.begin(9600);

  Wire.begin(ADDR_WHEEL);
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
  int pinPontH1 = stateMotors[id][2];
  int pinPontH2 = stateMotors[id][3];
  int pinPWM = stateMotors[id][4];
  int pinSensor = stateMotors[id][5];


  if (timerMotors[id][0] > millis()) {
    if (digitalRead(pinSensor) == HIGH && flagPulseMotors[id] == false) {
      flagPulseMotors[id] = true;
      nbPulseMotors[id]++;
    }
    if (digitalRead(pinSensor) == false && flagPulseMotors[id] == true) {
      flagPulseMotors[id] = false;
    }
  } else {
    timerMotors[id][0] = millis() + 20;
    stateMotors[id][1] = nbPulseMotors[id];//measure
    nbPulseMotors[id] = 0;
  }

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

  //int speedM = regulationPID(abs(value), stateMotors[id][1]);
  int speedM = abs(value);
  SoftPWMSetPercent(pinPWM, speedM);
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


#define KP 2
#define KI 2
#define KD 1

int integral = 0;
int previousError = 0;

int regulationPID(byte objective, byte measuredValue) {
  int error = objective - measuredValue;
  byte maxV = 100; //avant 255
  integral += (int)(error * 0.555);
  if (integral  > maxV) {
    integral = maxV;
  } else if (integral < -maxV) {
    integral = -maxV;
  }

  int derivative = 0;//(error - previousError) / durationPID;
  previousError = error;
  int value = (error * KP * 0.5f + integral * KI + derivative * KD) * 4;
  if ( value > maxV) {
    value = maxV;
  } else if ((error * KP * 0.5f + integral * KI + derivative * KD) * 4 < -maxV) {
    value = -maxV;
  }
  if (value < 0) {
    return 0;
  }
  return value;
}


////////////////////PARTIE I2C////////////////////
void receiveEvent(int howMany) {
  if (howMany == 0) {
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
  Serial.println("MotorId: " + String(motorId) + " motorValue: " + String(motorValue));
  if (!setMotorValue(motorId, motorValue)) {
    Serial.println("ERREUR MOTEUR");
  }
}
/////////////////////////////////////////////////////////////
