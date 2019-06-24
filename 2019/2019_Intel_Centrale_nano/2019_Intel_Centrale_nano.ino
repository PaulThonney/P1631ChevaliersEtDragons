/* Code de l'Arduino gérant l'intelligence centrale du Minotaure
  Son but est de récolter toutes les informations des capteurs et de prendre des décisions en conséquence
  @author: Dany VALADO (2018) Lucien PRUVOT Paul THONNEY
  DATE: 28.05.19
  REMERCIEMENTS: Merci à Maxime SCHARWATH et Joan MAILLARD pour leur aide
*/

#include <Wire.h> //I2C
//toutes les adresses I2C

#define ADDR_INTELLIGENCE_CENTRALE 0x0 // adresse de l'intelligence centrale, arduino nano sur le PCB Bluetooth
#define ADDR_TRACKING 0x10 //16 Arduino Nano se trouvant sur le PCB ADDR_TRACKING (ici c'est l'angle du servo qui est transmit)
#define ADDR_CONTACT 0x11 //17  PCB HMI, Nano se trouvant à gauche lorsqu'on regarde le U depuis sa base. Il gère les plaque de contact et les LED
#define ADDR_SOUND  0x12 //18
#define ADDR_WHEEL 0x13 //19 PCB Puissance, "Arduino 2" Nano
#define ADDR_EYES 0x14 //20

#define IS_MINOTAURE true

#define LOGS_DELAY 2000

//DEFINE ROBOT

#define MAX_LIFE 6
#define CONTACT_DEFAULT_MODE 0 //Valueur par defaut => 0: Tracking, 1: Rainbow, 2:AnimShield, 3: BlinkAll(RED)

//BUTTONS
#define BUFFER_SIZE 9
#define JOYSTICK_MARGIN 0.02f

//MANETTE STATES
#define CONNECTED 255

#define EASY 0
#define MEDIUM 1
#define HARD 2

#define MAX_SPEED 0
#define COOLDOWN 1

unsigned long timeStartAt;

bool ButtonA(bool flanc = false);
bool ButtonB(bool flanc = false);
bool ButtonX(bool flanc = false);
bool ButtonY(bool flanc = false);
bool ButtonWEST(bool flanc = false);
bool ButtonEAST(bool flanc = false);
bool ButtonNORTH(bool flanc = false);
bool ButtonSOUTH(bool flanc = false);
bool ButtonLB(bool flanc = false);
bool ButtonRB(bool flanc = false);
bool ButtonLS(bool flanc = false);
bool ButtonRS(bool flanc = false);
bool ButtonSTART(bool flanc = false);
bool ButtonSELECT(bool flanc = false);
bool flancsMontants[14];

byte modules[5] =  {ADDR_TRACKING, ADDR_CONTACT, ADDR_SOUND, ADDR_WHEEL, ADDR_EYES};
bool stateModules[5];

byte dataBuffer[BUFFER_SIZE];
byte controllerOutput = 0;
byte controllerBuzzer = 0; //0-15
byte controllerVibrator = 0; //0-15

typedef enum State { // On définit les états possible de la machine
  Automatique,
  Manuel,
  PauseGenerale,
  MenuSelection,
  MenuGO,
  Disconnected,
  Difficulty,
} State;

State setState(State state, int menuPos = -1);
State savedMode = State::Manuel;
State currentState = State::Automatique; // On démarre sur le menu de sélection
State previousState; // Ancien état

bool isStartedState = false;

int stateMenuPos = 0; // Position du curseur
unsigned long lastContactAt = 0;
unsigned long askResponseAt = 0;
unsigned long lastPingAt = 0;
bool waitingResponse = false;
unsigned long nbRequest;
unsigned long nbTransmission;
long loopTime;

bool sendEyes(int id, int data = -1);
bool sendSound(int id, int data = -1);
bool sendContact(int id, int data = -1, int duration = -1);

// maxSpeed[%] - hurtCooldown[ms]
int difficulty[][2] = {
  {30, 500},//easy
  {75, 2500},//medium
  {100, 1000}//hard
};
int currentDifficulty = 0;

int getDifficulty(int id) {
  return difficulty[currentDifficulty][id];
}

//AUTOMATIQUE
long lastUpdateHead = 0;
int headAngle = 0;
byte targetDistance = 0;
bool isFindTarget = false;

//ROBOT
byte currentLife = MAX_LIFE;
int hurtCooldown = 0;
int currentMotorValue[2];
//END ROBOT
unsigned long lastLogAt;

void setup() {
  Wire.begin(ADDR_INTELLIGENCE_CENTRALE);
  Serial.begin(115200);

  //Serial.println("Setup completed");
  delay(1000);
  pingModules();
  setupRobot();
  sendSound(0, 0);// Starting sound
}

void loop() {
  timeStartAt = millis();
  communicationController(); // On commence par communiquer les dernières infos avec la manette
  pingModules();
  loopAmbiant();
  if (millis() > askResponseAt + 25) {
    askResponseAt = 0;
    waitingResponse = false;
  }
  switch (currentState) {
    case State::Automatique: { // Mode automatique du robot
        loopAutomatique();
        break;
      }
    case State::Manuel: { // Mode manuel du robot
        loopManuel();
        break;
      }
    case State::PauseGenerale: { // mode menu de pause
        loopPauseGenerale();
        break;
      }
    case State::MenuSelection: { // Mode menu principal
        loopMenuSelection();
        break;
      }
    case State::MenuGO: { // Mode MenuGo
        loopMenuGo();
        break;
      }
    case State::Disconnected: { // Mode Disconnected
        loopDisconnected();
        break;
      }
    case State::Difficulty: { // Mode Difficulty
        loopDifficulty();
        break;
      }
  }
  loopTime = millis() - timeStartAt;

  logs();
}

void setupRobot() {
  currentLife = MAX_LIFE;
  hurtCooldown = 0;
  //sendTracking(0x2E);
  sendEyes(0);
  sendContact(CONTACT_DEFAULT_MODE);
  sendSound(250);//StopSound
}

int boostSpeed() {
  int percent = 0;
  if (isCooldown()) {
    percent += 50;
  }
  return percent;
}

void resumeGame() {
  sendEyes(0);
  sendContact(CONTACT_DEFAULT_MODE);
  sendSound(250);//StopSound
}

bool isCooldown() {
  return (millis() < hurtCooldown);
}

bool hurt(int dmg) {
  if (dmg <= 0)return false;
  //Serial.println("Cooldown:" + String(isCooldown()));
  if (millis() < hurtCooldown)return false; //Cooldown
  int cooldownDuration = getDifficulty(COOLDOWN);
  hurtCooldown = millis() + cooldownDuration;
  currentLife -= dmg;
  //Serial.println(String(dmg) + " dmg");
 // Serial.println(String(currentLife) + " lives");
  if (currentLife <= 0) {//DEAD
    currentLife = 0;
    die();
  } else {
    sendEyes(1);
    sendSound(1);//HURT
    sendContact(3, CONTACT_DEFAULT_MODE, (cooldownDuration / 250)); // blink pendant (x*250ms)
  }
  return true;
}

void die() {
  sendEyes(3);//eyeDead
  sendSound(3);//soundDead
  sendContact(4);//animDead
}

bool isDead() {
  return (currentLife <= 0);
}


void loopHurt() {
  if (millis() <= lastContactAt + 1000) {
    return;
  }
  lastContactAt = millis();
  byte buffer[10];
  if (getData(ADDR_CONTACT, buffer, 2)) {
    if ((bool)buffer[0] == true) {
      int dmg = 0;
     // Serial.println("Which Contact:" + String(buffer[1]));
      switch (buffer[1]) {
        case 0: dmg = 1; break;
        case 1: dmg = 1; break;
        case 2: dmg = 1; break;
        case 3: dmg = 1; break;
        case 4: dmg = 1; break;
        case 5: dmg = 1; break;
      }
      hurt(dmg);
    }
  }
}

void pingModules() {
  if (millis() <= lastPingAt + 500) {
    return;
  }
  lastPingAt = millis();
  for (int i = 0; i < sizeof(modules) / sizeof(modules[0]); i++) {
    bool state = pingAddr(modules[i]);
    if (state != stateModules[i]) {
     // Serial.println("Module " + String(i) + " " + String((state ? "CONNECTED" : "DISCONNECTED")));
      sendSound(0, state ? 1 : 2);
    }
    stateModules[i] = state;
  }
}

bool pingAddr(int addr) {
  if (waitingResponse)return false;
  nbTransmission++;
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

bool getData(int addr, byte *buffer, int nbBytes) {
  if (waitingResponse)return false;
  if (!pingAddr(addr))return false;
  nbRequest++;
  waitingResponse = true;
  Wire.requestFrom(addr, nbBytes); // Request the transmitted two bytes from the two registers
  if (Wire.available() <= nbBytes) {
    int i = 0;
    while (Wire.available()) {
      if (i >= nbBytes)break;
      buffer[i] = Wire.read();
      i++;
    }
    waitingResponse = false;
    return true;
  }
  return false;
}

bool sendData(int addr, byte *buffer, int nbBytes) {
  if (waitingResponse)return false;
  nbTransmission++;
  Wire.beginTransmission(addr);
  for (int i = 0; i < nbBytes; i++) {
    Wire.write(buffer[i]);
  }
  return Wire.endTransmission() == 1;
}

unsigned long lastAmbiantAt = 0;
void loopAmbiant() {
  if (isDead())return;
  if (IS_MINOTAURE) {
    if (millis() > lastAmbiantAt) {
      sendSound(4);//GROWL
      sendEyes(6);//ANGRY
      lastAmbiantAt = millis() + random(5000, 15000);
    }
  }
}

void loopDisconnected() {
  if (onStartState()) {
    sendEyes(7);
    sendMotorValue(0, 0);
    sendMotorValue(1, 0);
    sendSound(0, 4);
    sendContact(4);
  }
  if (InfoController() == CONNECTED) {
    setState(State::MenuGO);
    sendEyes(8);
    sendSound(0, 5);
    sendContact(CONTACT_DEFAULT_MODE);
  }
}

int getSpeed(int speed) {
  return ((100 + boostSpeed()) * speed) / 100.0;
}

/*
   Fonction qui gère le mode automatique des robots
   Pour le moment elle récupère uniquement l'angle du servo et le transmet aux roues.
*/
void loopAutomatique() {
  if (onStartState()) {//Seulement la première fois qu'il rentre dans la loop
    sendTracking(0x1E);
  }

  if (checkPause()) { // quitte directement la loop si la pause est pressée et évite que le "state" puisse être changé dans la fonction
    return;
  }

  loopHurt();

  if (isDead()) {
    setState(State::MenuSelection);
    return;
  }

  if (millis() - lastUpdateHead >  25) { //demande à la pixy ces valeurs toutes les 25ms

    byte buffer[10];
    if (getData(ADDR_TRACKING, buffer, 3)) {
      headAngle = map(buffer[0], 0, 180, -90, 90);
      targetDistance = buffer[3];
      isFindTarget = buffer[2];

      waitingResponse = false;
      lastUpdateHead = millis();
    }
    //sendEyes(5, (3 << 3) | map(headAngle, -90, 90, 0, 6));
  }

  if (headAngle > -5 && headAngle < 5) {
    if (isFindTarget) {
      int speed = getSpeed(map(targetDistance, 0, 255, 10, getDifficulty(MAX_SPEED)));
      sendMotorValue(0, speed);
      sendMotorValue(1, speed);
    }

  } else {
    int speed = getSpeed(map(abs(headAngle), 0, 90, 10, getDifficulty(MAX_SPEED)));
    if (headAngle < 0) {
      sendMotorValue(0, -speed);
      sendMotorValue(1, speed);
    } else {
      sendMotorValue(0, speed);
      sendMotorValue(1, -speed);
    }
  }
  controllerOutput = 26;
}

/*
   Fonction qui gère le mode manuel des robots
   Elle transmet la position des joysticks a l'arduino des roues
*/
void loopManuel() {
  if (onStartState()) {//Seulement la première fois qu'il rentre dans la loop
    sendTracking(0x2E);
  }

  //sendEyes(5, (map(AxisLX(), 0, 255, 0, 6) << 3) | map(AxisLY(), 0, 255, 0, 6));

  if (checkPause()) { // quitte directement la loop si la pause est pressée et évite que le "state" puisse être changé dans la fonction
    return;
  }

  loopHurt();

  if (isDead()) {
    setState(State::MenuSelection);
  }


  controllerOutput = 27;

  float jX = JoystickValue(AxisLX());
  float jY = JoystickValue(AxisLY());

  float hyp = sqrt(jX * jX + jY * jY);

  int speed1 = getDifficulty(MAX_SPEED) * hyp;
  int speed2 = getDifficulty(MAX_SPEED) * hyp;

  if (jX < 0) {
    speed1 *= (1 - abs(jX));
  }
  if (jX > 0) {
    speed2 *= (1 - abs(jX));
  }

  sendMotorValue(0, getSpeed(speed1));
  sendMotorValue(1, getSpeed(speed2));
}

bool sendSound(int id, int data) {
  if (waitingResponse || !stateModules[2])return false;
  nbTransmission++;
  Wire.beginTransmission(ADDR_SOUND);
  Wire.write(id);
  if (data > -1) {
    Wire.write(data);
  }
  return Wire.endTransmission() == 1;

}

bool sendContact(int id, int data, int duration) {
  if (waitingResponse || !stateModules[1])return false;
  nbTransmission++;
  Wire.beginTransmission(ADDR_CONTACT);
  Wire.write(id);
  if (data > -1) {
    Wire.write(data);
  }
  if (duration > -1) {
    Wire.write(duration);
  }
  return Wire.endTransmission() == 1;

}

bool sendTracking(int id) {
  if (waitingResponse || !stateModules[0])return false;
  nbTransmission++;
  Wire.beginTransmission(ADDR_TRACKING);
  Wire.write(id);
  return Wire.endTransmission() == 1;

}

bool sendEyes(int id, int data) {
  if (waitingResponse || !stateModules[4])return false;
  nbTransmission++;
  Wire.beginTransmission(ADDR_EYES);
  Wire.write(id);
  if (data > -1) {
    Wire.write(data);
  }
  return Wire.endTransmission() == 1;
}

bool sendMotorValue(byte id, int value) {
  if (waitingResponse || !stateModules[3])return false;
  if (currentMotorValue[id] == value)return true; //évite de faire une comm si rien n'a changé
  currentMotorValue[id] = value;
  byte data = abs(value);
  if ( data > 100)data = 100;
  if ((value < 0)) {
    bitSet(data, 7);
  }
  nbTransmission++;
  Wire.beginTransmission(ADDR_WHEEL);
  Wire.write(id);
  Wire.write(data);
  return Wire.endTransmission() == 1;
}

/*
   @func void communicationController Gère la communication avec l'esp32 du groupe manette installé sur le pcb.
   On commence par réenvoyer les données que l'on possède ce qui fait que l'esp32 nous envoieles siennes directement.
   @param null
   @return void
*/
void communicationController() {
  byte dataBufferWrite[2] = {controllerOutput, (byte)((controllerBuzzer << 4) | controllerVibrator)}; // réenvoie les données à la manette
  Serial.write(dataBufferWrite, 2);
  // controlle la longueure de la tramme
  while (Serial.available() < BUFFER_SIZE) {}
  Serial.readBytes(dataBuffer, BUFFER_SIZE); //lit les infos en provenance de la manette

  if (InfoController() != CONNECTED) {
    setState(State::Disconnected);
  }
}
/*
   @func State setState  Elle change l'état actuelle de la variable state et retourne son état actuel.
   Permet de faire d'autres actions sur des variables lors d'un changement d'état directement dans cette fonction si nécessaire.
   Evite de passer par une variable grobale.
   @param State state
   #param int menuPos
   @return State currentState
*/
State setState(State state, int menuPos) {
  if (menuPos > -1) {
    stateMenuPos = menuPos;
  }
  if (currentState == state)return currentState; // Evite de traiter inutilement les données s'il n'y a pas de changement
  isStartedState = false;
  previousState = currentState;
  currentState = state;
  return currentState;
}

bool onStartState() {
  if (!isStartedState) {
    isStartedState = true;
    return true;
  }
  return false;
}
/*
   @func bool checkPause Vérifie s'il y a une demande de pause lors de la partie
   @param null
   @return bool
*/
bool checkPause() {
  if (ButtonB(true)) {
    sendMotorValue(0, 0);
    sendMotorValue(1, 0);
    setState(State::PauseGenerale, 0);
    return true;
  }
  return false;
}
/*
   @func void loopMenuGo Gère le menuGo (affichage et pression des boutons)
   @param null
   @return void
*/
void  loopMenuGo() {
  if (onStartState()) {
  }
  controllerOutput = 3;
  if (ButtonA(true)) {
    setState(savedMode);
  }
}
/*
   @param changeCursorPosition Change la position du curseur dans les menus à l'aide des touches NORTH et SOUTH
   @param byte sizeMenu taille du menu en comptant à partir de 1
   @return byte stateMenuPos position du curseur
*/
byte changeCursorPosition(byte sizeMenu) {
  sizeMenu--; // permet de donner taille menu en comptant à partir de 1
  if (ButtonNORTH(true)) {
    stateMenuPos++;
    if (stateMenuPos > sizeMenu) {
      stateMenuPos = 0;
    }
  }

  if (ButtonSOUTH(true)) {
    stateMenuPos--;
    if (stateMenuPos < 0) {
      stateMenuPos = sizeMenu;
    }
  }
  return stateMenuPos;
}
/*
   @func void loopPauseGenerale Gère le mode pauseGenerale (affichage et pression des boutons)
   @param null
   @return void
*/
void loopPauseGenerale() {
  if (onStartState()) {//Seulement la première fois qu'il rentre dans la loop
    sendSound(0, 3);//Musique Pause
    sendEyes(4);// Yeux Pause
  }

  byte tailleMenu = 2; // compte à partir de 1 donc ici le menu fait 2
  byte pos = changeCursorPosition(tailleMenu);//changement position curseur

  switch (pos) {
    case 0:
      controllerOutput = 28; // Info d'affichage pour la manette
      if (ButtonA(true)) {
        resumeGame();
        setState(State::MenuGO);
      }
      break;
    case 1:
      controllerOutput = 29; // Info d'affichage pour la manette
      if (ButtonA(true)) {
        resumeGame();
        setState(State::MenuSelection);
      }
      break;
  }
}
/*
   @func void loopMenuSelection Gère le mode MenuSelection (affichage et pression des boutons)
   @param null
   @return void
*/
void  loopMenuSelection() {
  if (onStartState()) {
  }
  byte tailleMenu = 2;
  byte pos = changeCursorPosition(tailleMenu);//changement position curseur
  State selectedMode = State::Automatique;

  switch (pos) {
    default:
    case 0:
      controllerOutput = 1;
      selectedMode = State::Automatique;
      break;
    case 1:
      controllerOutput = 2;
      selectedMode = State::Manuel;
      break;
  }

  if (ButtonA(true)) {
    savedMode = selectedMode; // Lors du choix d'un mode on le stock pour que le menu pause puisse reprendre sur le bon mode
    setState(State::MenuGO);
  }
}

void loopDifficulty() {
  if (onStartState()) {
  }
  byte tailleMenu = 3;
  byte pos = changeCursorPosition(tailleMenu);//changement position curseur
  int diffTemp = -1;

  switch (pos) {
    default:
    case 0:
      controllerOutput = 1;
      diffTemp = EASY;
      break;
    case 1:
      controllerOutput = 2;
      diffTemp = MEDIUM;
      break;
    case 3:
      controllerOutput = 2;
      diffTemp = HARD;
      break;
  }

  if (ButtonA(true)) {
    if (diffTemp > -1)
      currentDifficulty = diffTemp;
    setState(State::MenuSelection);
  }
  if (ButtonB(true)) {//RETOUR
    setState(State::MenuSelection);
  }
}

//LOGS

void logs() {
  if (millis() < lastLogAt + LOGS_DELAY) {
    return;
  }
  lastLogAt = millis();
 /* Serial.println("======LOGS======");
  Serial.println("Execute Time: " + String(millis() / 1000.0) + "s");
  Serial.println("nbTransmission: " + String(nbTransmission));
  Serial.println("nbRequest: " + String(nbRequest));
  Serial.println("loopTime: " + String(loopTime) + "ms");
  Serial.println("currentState: " + String(currentState));
  Serial.println("headAngle: " + String(headAngle));
  Serial.println("targetDistance: " + String(targetDistance));
  Serial.println("isFindTarget: " + String(isFindTarget));
  Serial.println("currentDifficulty: " + String(currentDifficulty));
  Serial.println("currentMotorValue 1: " + String(currentMotorValue[0]) + "%");
  Serial.println("currentMotorValue 2: " + String(currentMotorValue[1]) + "%");
  Serial.println("Module Tracking (" + toHex(modules[0]) + "): " + String((stateModules[0] ? "CONNECTED" : "DISCONNECTED")));
  Serial.println("Module Contact (" + toHex(modules[1]) + "): " + String((stateModules[1] ? "CONNECTED" : "DISCONNECTED")));
  Serial.println("Module Sound (" + toHex(modules[2]) + "): " + String((stateModules[2] ? "CONNECTED" : "DISCONNECTED")));
  Serial.println("Module Wheel (" + toHex(modules[3]) + "): " + String((stateModules[3] ? "CONNECTED" : "DISCONNECTED")));
  Serial.println("Module Eyes (" + toHex(modules[4]) + "): " + String((stateModules[4] ? "CONNECTED" : "DISCONNECTED")));
  Serial.println("================");
  Serial.println();
  */
}


//CONTROLLER


float JoystickValue(byte v) {
  float tmp = mapfloat(v, 0, 255, -1, 1);
  if (tmp >= -JOYSTICK_MARGIN && tmp <= JOYSTICK_MARGIN)tmp = 0;
  return tmp;
}

float TriggerValue(byte v) {
  float tmp = mapfloat(v, 0, 255, 0, 1);
  return tmp;
}

/*
   @func byte InfoController retourne la valeure de l'etat de la mannette
   @param null
   @return byte
*/
byte InfoController() {
  return dataBuffer[8];
}

/*
   @func byte AxisLX retourne la valeure de l'axe X du joystick gauche
   @param null
   @return byte
*/
byte AxisLX() {
  return dataBuffer[4];
}
/*
   @func byte AxisLY retourne la valeure de l'axe Y du joystick gauche
   @param null
   @return byte
*/
byte AxisLY() {
  return dataBuffer[5];
}
/*
   @func byte AxisRX retourne la valeure de l'axe X du joystick droite
   @param null
   @return byte
*/
byte AxisRX() {
  return dataBuffer[6];
}
/*
   @func byte AxisRY retourne la valeure de l'axe Y du joystick droite
   @param null
   @return byte
*/
byte AxisRY() {
  return dataBuffer[7];
}
/*
   @func byte AxisLT retourne la valeure de la gachette droite
   @param null
   @return byte
*/
byte AxisLT() {
  return dataBuffer[2];
}
/*
   @func byte AxisRT retourne la valeure de la gachette Gauche
   @param null
   @return byte
*/
byte AxisRT() {
  return dataBuffer[3];
}
/*
   @func bool ButtonA retourne la valeure du boutton A
   @param null
   @return bool
*/
bool ButtonA(bool flanc) {
  bool v = bitRead(dataBuffer[0], 7);
  if (flanc)v = ButtonFlanc(v, 0);
  return v;
}
/*
   @func bool ButtonB retourne la valeure du boutton B
   @param null
   @return bool
*/
bool ButtonB(bool flanc) {
  bool v = bitRead(dataBuffer[0], 4);
  if (flanc)v = ButtonFlanc(v, 1);
  return v;
}
/*
   @func bool ButtonX retourne la valeure du boutton X
   @param null
   @return bool
*/
bool ButtonX(bool flanc) {
  bool v = bitRead(dataBuffer[0], 6);
  if (flanc)v = ButtonFlanc(v, 2);
  return v;
}
/*
   @func bool ButtonY retourne la valeure du boutton Y
   @param null
   @return bool
*/
bool ButtonY(bool flanc) {
  bool v = bitRead(dataBuffer[0], 5);
  if (flanc)v = ButtonFlanc(v, 3);
  return v;
}
/*
   @func bool ButtonWEST retourne la valeure du boutton WEST
   @param null
   @return bool
*/
bool ButtonWEST(bool flanc) {
  bool v = bitRead(dataBuffer[1], 4);
  if (flanc)v = ButtonFlanc(v, 4);
  return v;
}
/*
   @func bool ButtonEAST retourne la valeure du boutton EAST
   @param null
   @return bool
*/
bool ButtonEAST(bool flanc) {
  bool v = bitRead(dataBuffer[1], 6);
  if (flanc)v = ButtonFlanc(v, 5);
  return v;
}
/*
   @func bool ButtonNORTH retourne la valeure du boutton NORTH
   @param null
   @return bool
*/
bool ButtonNORTH(bool flanc) {
  bool v = bitRead(dataBuffer[1], 5);
  if (flanc)v = ButtonFlanc(v, 6);
  return v;
}
/*
   @func bool ButtonSOUTH retourne la valeure du boutton SOUTH
   @param null
   @return bool
*/
bool ButtonSOUTH(bool flanc) {
  bool v = bitRead(dataBuffer[1], 7);
  if (flanc)v = ButtonFlanc(v, 7);
  return v;
}
/*
   @func bool ButtonLB retourne la valeure du boutton LB
   @param null
   @return bool
*/
bool ButtonLB(bool flanc) {
  bool v = bitRead(dataBuffer[0], 2);
  if (flanc)v = ButtonFlanc(v, 8);
  return v;
}
/*
   @func bool ButtonRB retourne la valeure du boutton RB
   @param null
   @return bool
*/
bool ButtonRB(bool flanc) {
  bool v = bitRead(dataBuffer[0], 3);
  if (flanc)v = ButtonFlanc(v, 9);
  return v;
}
/*
   @func bool ButtonLS retourne la valeure du boutton LS
   @param null
   @return bool
*/
bool ButtonLS(bool flanc) {
  bool v = bitRead(dataBuffer[0], 4);
  if (flanc)v = ButtonFlanc(v, 10);
  return v;
}
/*
   @func bool ButtonRS retourne la valeure du boutton RS
   @param null
   @return bool
*/
bool ButtonRS(bool flanc) {
  bool v = bitRead(dataBuffer[0], 3);
  if (flanc)v = ButtonFlanc(v, 11);
  return v;
}
/*
   @func bool ButtonSTART retourne la valeure du boutton START
   @param null
   @return bool
*/
bool ButtonSTART(bool flanc) {
  bool v = bitRead(dataBuffer[0], 1);
  if (flanc)v = ButtonFlanc(v, 12);
  return v;
}
/*
   @func bool ButtonSELECT retourne la valeure du boutton SELECT
   @param null
   @return bool
*/
bool ButtonSELECT(bool flanc) {
  bool v = bitRead(dataBuffer[0], 0);
  if (flanc)v = ButtonFlanc(v, 13);
  return v;
}
/*
   @func bool ButtonFlanc Permet de détecter les flancs montants des boutons dans une seule fonction
   @param bool button
   #param byte flancId 0: null; 1: A; 2: B, 3: NORTH, 4: SOUTH
   @return bool
*/
bool ButtonFlanc(bool button, byte flancId) {
  bool temp = false;
  if (button && !flancsMontants[flancId]) {
    temp = true;
  }
  flancsMontants[flancId] = button;
  return temp;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

String toHex(int num) {
  char buffer[10];
  sprintf(buffer, "%x", num);
  return "0x" + String(buffer);
}
