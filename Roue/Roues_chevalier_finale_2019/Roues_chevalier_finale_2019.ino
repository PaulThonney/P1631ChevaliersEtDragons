#include <SoftPWM.h>

#include <Wire.h>

#define PWM_OUTPUT_MOTOR_L 4 // Pin 5 où sort le pwm du moteur 2
#define INPUT_4_MOTOR_L    3  // Pin 9 pour un sens
#define INPUT_3_MOTOR_L    9  // Pin 3 pour l'autre sens

#define PWM_OUTPUT_MOTOR_R 7 // Pin 6 où sort le pwm du moteur 1
#define INPUT_2_MOTOR_R    10  // Pin 10 pour un sens
#define INPUT_1_MOTOR_R    11  // Pin 11 pour l'autre sens

#define PIN_TEMP_R    A2
#define PIN_TEMP_L    A3
#define PIN_VENT_R    12
#define PIN_VENT_L    8
#define PIN_LED_URGENCE 2
#define EMERGENCY_TEMP 29
#define PWM_MIN 10
#define PWM_MAX 100

bool tempOverheating[] = {0, 0};

int stateMotors[][4] = {
  {-10, INPUT_1_MOTOR_R, INPUT_2_MOTOR_R, PWM_OUTPUT_MOTOR_R},
  {10, INPUT_3_MOTOR_L, INPUT_4_MOTOR_L, PWM_OUTPUT_MOTOR_L}
}; //value, ph1, ph2, pwm

void setup() {
  SoftPWMBegin();
  Serial.begin(9600);

  Wire.begin(19);
  Wire.onReceive(receiveEvent);

  //Moteur droite
  SoftPWMSet(PWM_OUTPUT_MOTOR_R, 0);
  pinMode(INPUT_1_MOTOR_R,         OUTPUT);
  pinMode(INPUT_2_MOTOR_R,         OUTPUT);
  //Input 1 et 2 : pont en H du moteur droite

  //Moteur gauche
  SoftPWMSet(PWM_OUTPUT_MOTOR_L, 0);
  pinMode(INPUT_3_MOTOR_L,         OUTPUT);
  pinMode(INPUT_4_MOTOR_L,         OUTPUT);

  digitalWrite(INPUT_1_MOTOR_R,    LOW);
  digitalWrite(INPUT_2_MOTOR_R,    LOW);
  digitalWrite(INPUT_3_MOTOR_L,    LOW);
  digitalWrite(INPUT_4_MOTOR_L,    LOW);


  pinMode (PIN_LED_URGENCE, OUTPUT);
  digitalWrite(PIN_LED_URGENCE, LOW); //Eteint la led d'urgence
  pinMode (4, INPUT);//désactivation de pin
  pinMode (5, INPUT);//désactivation de pin
}

void checkTemp() {
  int tempR = ( analogRead(PIN_TEMP_R) / 1024.0) * 5000 / 10;
  if (tempR > EMERGENCY_TEMP) {
    tempOverheating[0] = true;
    digitalWrite(PIN_VENT_R, HIGH);
  }
  else {
    tempOverheating[0] = false;
    digitalWrite(PIN_VENT_R, LOW);
  }

  int tempL = ( analogRead(PIN_TEMP_L) / 1024.0) * 5000 / 10;
  if (tempL > EMERGENCY_TEMP) {
    tempOverheating[1] = true;
    digitalWrite(PIN_VENT_L, HIGH);
  }
  else {
    tempOverheating[1] = false;
    digitalWrite(PIN_VENT_L, LOW);
  }

  //GESTION DE LA LED
  if (tempOverheating[0] && tempOverheating[1]) {
    digitalWrite(PIN_LED_URGENCE, (millis() % 250 > 125));
  }
  else if (tempOverheating[0] != tempOverheating[1]) {
    digitalWrite(PIN_LED_URGENCE, (millis() % 500 > 250));
  }
  else {
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


void loopMotor() {
  Motor(0);
  Motor(1);
}

void loop() {
  checkTemp();
  loopMotor();
}

////////////////////PARTIE I2C////////////////////
void receiveEvent(int howMany) {
  /*//Serial.println("howMany: ");
    //Serial.print(howMany);*/
  static bool separatorSeen = false;
  uint32_t posX = 0, posY = 0;

  for (int i = 0; i < howMany; i++) {
    if (i < 2) {
      posX <<= 8;
      posX |= (uint8_t)Wire.read();
    } else {
      posY <<= 8;
      posY |= (uint8_t)Wire.read();
    }  Serial.println (posX);
  }
}
/////////////////////////////////////////////////////////////
