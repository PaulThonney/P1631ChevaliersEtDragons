
/* Code de l'Arduino gérant l'intelligence centrale du Minotaure
  Son but est de récolter toutes les informations des capteurs et de prendre des décisions en conséquence
  @author: Dany VALADO (2018) Lucien PRUVOT Paul THONNEY
  DATE: 28.05.19
  REMERCIEMENTS: Merci à Maxime SCHARWATH et Joan MAILLARD pour leur aide
*/

#include <Wire.h> //I2C
//toutes les adresses I2C

#define ADRESSE_INTELLIGENCE_CENTRALE 100 // adresse de l'intelligence centrale, arduino nano sur le PCB Bluetooth
#define ADDR_TRAQUAGE 20 // Arduino Nano se trouvant sur le PCB ADDR_TRAQUAGE (ici c'est l'angle du servo qui est transmit)
#define CONTACT 2  //  PCB HMI, Nano se trouvant à gauche lorsqu'on regarde le U depuis sa base. Il gère les plaque de contact et les LED
#define ADRESSE_ROUE 19// PCB Puissance, "Arduino 2" Nano
#define SON // PCB HMI,  Nano se trouvant à droite lorsqu'on regarde le U depuis sa base. Il gère le HP
#define ADDR_EYES 69
//DEFINE ROBOT

#define MAX_LIFE 6
#define HURT_COOLDOWN 5000 // en ms

//BUTTONS
#define BUFFER_SIZE 8
#define A 7 //addr 0
#define B 4 //addr 0
#define X 6 //addr 0
#define Y 5 //addr 0
#define WEST 4 //addr 1
#define EAST 6 //addr 1
#define NORTH 5 //addr 1
#define SOUTH 7 //addr 1
#define LB 2 //addr 0
#define RB 3 //addr 0
#define LS 4 //addr 1
#define RS 3 //addr 1
#define START 1 //addr 0
#define SELECT 0 //addr 0
#define JOYSTICK_MARGIN 0.02f

byte dataBuffer[BUFFER_SIZE];
byte output = 0;
//Il stoque la valeure actuelle du flanc montant pour les boutons dont la correspondance est telle:
//0: null; 1: A; 2: B, 3: NORTH, 4: SOUTH
bool flancsMontants[] = {false, false, false, false, false};
byte anglePixy;
byte sonEtVibreur; //4 premiers bits: buzzer; 4 derniers bits: vibreur

typedef enum State { // On définit les états possible de la machine
  Automatique,
  Manuel,
  PauseGenerale,
  MenuSelection,
  MenuGO,
} State;

State setState(State state, int menuPos = -1);
State savedMode = State::Automatique;
State currentState = State::MenuSelection; // On démarre sur le menu de sélection
State previousState; // Ancien état

bool isStartedState = false;

int stateMenuPos = 0; // Position du curseur

unsigned long askResponseAt;
bool waitingResponse = false;



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

void setup() {
  Wire.begin(ADRESSE_INTELLIGENCE_CENTRALE);
  Wire.onReceive(receiveEvent);
  Serial2.begin(115200);
  Serial.begin(115200);
  Serial.println("Setup completed");
}

void loop() {
  communicationManette(); // On commence par communiquer les dernières infos avec la manette
  //Serial.println(waitingResponse);
  if (millis() > askResponseAt + 25) {
    askResponseAt = 0;
    waitingResponse = false;
  }
  //return;
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
  }
}

void setupRobot() {
  currentLife = MAX_LIFE;
  hurtCooldown = 0;
}

bool hurt(byte dmg) {
  if (millis() < hurtCooldown)return false; //Cooldown
  hurtCooldown = millis() + HURT_COOLDOWN;
  currentLife -= dmg;
  if (currentLife <= 0) {
    //DEAD
    currentLife = 0;
  }
  return true;
}

/*
   Fonction qui gère le mode automatique des robots
   Pour le moment elle récupère uniquement l'angle du servo et le transmet aux roues.
*/
void loopAutomatique() {
  if (onStartState()) {//Seulement la première fois qu'il rentre dans la loop
    if (!waitingResponse) {
      Wire.beginTransmission(ADDR_TRAQUAGE);
      Wire.write(0x1E);
      Wire.endTransmission();
    }
  }

  if (checkPause()) { // quitte directement la loop si la pause est pressée et évite que le "state" puisse être changé dans la fonction
    return;
  }

  if (millis() > lastUpdateHead + 100) {//demande à la pixy ces valeurs toutes les 25ms
    if (!waitingResponse) {
      waitingResponse = true;
      Wire.requestFrom(ADDR_TRAQUAGE, 3);   // request 6 bytes from slave device #8
      uint8_t i = 0;
      uint8_t rawData[3] = {0, 0, 0};
      while (Wire.available()) {
        byte b = Wire.read();
        Serial.println(String(i) + " " + String(b));
        switch (i) {
          case 0: headAngle = map(b, 0, 180, -90, 90); break;
          case 1: targetDistance = b; break;
          case 2: isFindTarget = b; break;
        }
        i++;
      }
      waitingResponse = false;
      Wire.endTransmission();
      lastUpdateHead = millis();
      Serial.println("Servo: " + String(headAngle) + " Distance: " + String(targetDistance) + " isTracking: " + String(isFindTarget));

    }
    if (!waitingResponse) {
      Wire.beginTransmission(ADDR_EYES);
      Wire.write(5);
      Wire.write((3 << 3) | map(headAngle, -90, 90, 0, 6));
      Wire.endTransmission();
    }
  }

  //headAngle;
  //targetDistance;
  //isFindTarget;


  output = 26;
}

/*
   Fonction qui gère le mode manuel des robots
   Elle transmet la position des joysticks a l'arduino des roues
*/
void loopManuel() {
  if (onStartState()) {//Seulement la première fois qu'il rentre dans la loop

    if (!waitingResponse) {
      Wire.beginTransmission(ADDR_TRAQUAGE);
      Wire.write(0x2E);
      Wire.endTransmission();
    }

  }

  if (!waitingResponse) {
    Wire.beginTransmission(ADDR_EYES);
    Wire.write(5);
    Wire.write((map(AxisLX(), 0, 255, 0, 6) << 3) | map(AxisLY(), 0, 255, 0, 6));
    Wire.endTransmission();
  }

  if (checkPause()) { // quitte directement la loop si la pause est pressée et évite que le "state" puisse être changé dans la fonction
    return;
  }
  output = 27;

  float jX = JoystickValue(AxisLX());
  float jY = JoystickValue(AxisLY());


  float hyp = sqrt(jX * jX + jY * jY);

  int speed1 = 100 * hyp;
  int speed2 = 100 * hyp;

  if (jX < 0) {
    speed1 *= (1 - abs(jX));
  }
  if (jX > 0) {
    speed2 *= (1 - abs(jX));
  }

  sendMotorValue(0, speed1);
  sendMotorValue(1, speed2);
  //Serial.println("speed1:" + String(speed1));
  //Serial.println("speed2:" + String(speed2));
}

void sendMotorValue(byte id, int value) {
  if (currentMotorValue[id] == value)return; //évite de faire une comm si rien n'a changé
  currentMotorValue[id] = value;
  byte data = abs(value);
  if ((value < 0)) {
    bitSet(data, 7);
  }
  if (!waitingResponse) {
    Wire.beginTransmission(ADRESSE_ROUE);
    Wire.write(id);
    Wire.write(data);
    Wire.endTransmission();
  }
}

/*
   @func byte AxisLX retourne la valeure de l'axe X du joystick gauche
   @param null
   @return byte
*/
void receiveEvent(int howMany) {
  Serial.println("howMany: " + String(howMany));

  waitingResponse = false;
  Serial.println(String(millis() - askResponseAt) + "ms");

  byte addr = Wire.read();
  switch (addr) {
    case ADDR_TRAQUAGE:

      break;
  }
}

float JoystickValue(byte v) {
  float tmp = mapfloat(v, 0, 255, -1, 1);
  if (tmp >= -JOYSTICK_MARGIN && tmp <= JOYSTICK_MARGIN)tmp = 0;
  return tmp;
}

/*
   @func byte AxisLX retourne la valeure de l'axe X du joystick gauche
   @param null
   @return byte
*/
byte AxisLX()       {
  return dataBuffer[4];
}
/*
   @func byte AxisLY retourne la valeure de l'axe Y du joystick gauche
   @param null
   @return byte
*/
byte AxisLY()       {
  return dataBuffer[5];
}
/*
   @func byte AxisRX retourne la valeure de l'axe X du joystick droite
   @param null
   @return byte
*/
byte AxisRX()       {
  return dataBuffer[6];
}
/*
   @func byte AxisRY retourne la valeure de l'axe Y du joystick droite
   @param null
   @return byte
*/
byte AxisRY()       {
  return dataBuffer[7];
}
/*
   @func byte AxisLT retourne la valeure de la gachette droite
   @param null
   @return byte
*/
byte AxisLT()       {
  return dataBuffer[2];
}
/*
   @func byte AxisRT retourne la valeure de la gachette Gauche
   @param null
   @return byte
*/
byte AxisRT()       {
  return dataBuffer[3];
}
/*
   @func bool ButtonA retourne la valeure du boutton A
   @param null
   @return bool
*/
bool ButtonA()      {
  return bitRead(dataBuffer[0], A);
}
/*
   @func bool ButtonB retourne la valeure du boutton B
   @param null
   @return bool
*/
bool ButtonB()      {
  return bitRead(dataBuffer[0], B);
}
/*
   @func bool ButtonX retourne la valeure du boutton X
   @param null
   @return bool
*/
bool ButtonX()      {
  return bitRead(dataBuffer[0], X);
}
/*
   @func bool ButtonY retourne la valeure du boutton Y
   @param null
   @return bool
*/
bool ButtonY()      {
  return bitRead(dataBuffer[0], Y);
}
/*
   @func bool ButtonWEST retourne la valeure du boutton WEST
   @param null
   @return bool
*/
bool ButtonWEST()   {
  return bitRead(dataBuffer[1], WEST);
}
/*
   @func bool ButtonEAST retourne la valeure du boutton EAST
   @param null
   @return bool
*/
bool ButtonEAST()   {
  return bitRead(dataBuffer[1], EAST);
}
/*
   @func bool ButtonNORTH retourne la valeure du boutton NORTH
   @param null
   @return bool
*/
bool ButtonNORTH()  {

  return bitRead(dataBuffer[1], NORTH);
}
/*
   @func bool ButtonSOUTH retourne la valeure du boutton SOUTH
   @param null
   @return bool
*/
bool ButtonSOUTH()  {
  return bitRead(dataBuffer[1], SOUTH);
}
/*
   @func bool ButtonLB retourne la valeure du boutton LB
   @param null
   @return bool
*/
bool ButtonLB()     {
  return bitRead(dataBuffer[0], LB);
}
/*
   @func bool ButtonRB retourne la valeure du boutton RB
   @param null
   @return bool
*/
bool ButtonRB()     {
  return bitRead(dataBuffer[0], RB);
}
/*
   @func bool ButtonLS retourne la valeure du boutton LS
   @param null
   @return bool
*/
bool ButtonLS()     {
  return bitRead(dataBuffer[0], LS);
}
/*
   @func bool ButtonRS retourne la valeure du boutton RS
   @param null
   @return bool
*/
bool ButtonRS()     {
  return bitRead(dataBuffer[0], RS);
}
/*
   @func bool ButtonSTART retourne la valeure du boutton START
   @param null
   @return bool
*/
bool ButtonSTART()  {
  return bitRead(dataBuffer[0], START);
}
/*
   @func bool ButtonSELECT retourne la valeure du boutton SELECT
   @param null
   @return bool
*/
bool ButtonSELECT() {
  return bitRead(dataBuffer[0], SELECT);
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
    //Serial.println("BUTTON PRESSED: " + String(flancId));
    temp = true;
  }
  flancsMontants[flancId] = button;
  return temp;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*
   @func void communicationManette Gère la communication avec l'esp32 du groupe manette installé sur le pcb.
   On commence par réenvoyer les données que l'on possède ce qui fait que l'esp32 nous envoieles siennes directement.
   @param null
   @return void
*/
void communicationManette() {
  uint8_t dataBufferWrite[2] = {output, sonEtVibreur};// réenvoie les données à la manette
  Serial2.write(dataBufferWrite, 2);
  while (Serial2.available() < 8) { // controlle la longueure de la tramme
    //Serial.print("#");
  }
  Serial2.readBytes(dataBuffer, BUFFER_SIZE); //lit les infos en provenance de la manette
}
/*
   @func State setState  Elle change l'état actuelle de la variable state et retourne son état actuel.
   Permet de faire d'autres actions sur des variables lors d'un changement d'état directement dans cette fonction si nécessaire.
   Evite de passer par une variable grobale.
   @param State state
   #param int menuPos
   @return State currentState
*/
State setState(State state, int menuPos = -1) {
  if (menuPos > -1) {
    stateMenuPos = menuPos;
  }
  /*
    Serial.print("Set State: ");
    Serial.print(state);
    Serial.print(" Saved: ");
    Serial.print(savedMode);
    Serial.print(" MenuPos: ");
    Serial.print(stateMenuPos);
    Serial.println();
  */
  //Serial.println("Set State: " + String(state));
  if (currentState == state)return currentState; // Evite de traiter inutilement les données s'il n'y a pas de changement
  isStartedState = false;
  //previousState = currentState; // non utilisé car remplacé par le savedMode
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
  if (ButtonFlanc(ButtonB(), 2)) {
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
  output = 3;
  if (ButtonFlanc(ButtonA(), 1)) {
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
  if (ButtonFlanc(ButtonNORTH(), 3)) {
    stateMenuPos++;
    if (stateMenuPos > sizeMenu) {
      stateMenuPos = 0;
    }
  }

  if (ButtonFlanc(ButtonSOUTH(), 4)) {
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
  byte tailleMenu = 2; // compte à partir de 1 donc ici le menu fait 2
  byte pos = changeCursorPosition(tailleMenu);//changement position curseur

  switch (pos) {
    case 0:
      output = 28; // Info d'affichage pour la manette

      if (ButtonFlanc(ButtonA(), 1)) {
        setState(State::MenuGO);
      }
      break;
    case 1:
      output = 29; // Info d'affichage pour la manette

      if (ButtonFlanc(ButtonA(), 1)) {
        setState(State::MenuSelection, 0);
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
  byte tailleMenu = 2;
  byte pos = changeCursorPosition(tailleMenu);//changement position curseur
  State selectedMode = State::Automatique;

  switch (pos) {
    default:
    case 0:
      output = 1;
      selectedMode = State::Automatique;
      break;
    case 1:
      output = 2;
      selectedMode = State::Manuel;
      break;
  }

  if (ButtonFlanc(ButtonA(), 1)) {
    savedMode = selectedMode; // Lors du choix d'un mode on le stock pour que le menu pause puisse reprendre sur le bon mode
    setState(State::MenuGO);
  }
}
