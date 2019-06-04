/* Code de l'Arduino gérant l'intelligence centrale du Minotaure
  Son but est de récolter toutes les informations des capteurs et de prendre des décisions en conséquence
  AUTEURS: Dany VALADO (2018) Lucien PRUVOT Paul THONNEY
  DATE: 28.05.19
  REMERCIEMENTS: Merci à Maxime SCHARWATH et Joan MAILLARD pour leur aide
*/

#include <Wire.h> //I2C

//toutes les adresses I2C

#define ADRESSE_INTELLIGENCE_CENTRALE 1 // adresse de l'intelligence centrale, arduino nano sur le PCB Bluetooth

#define TRAQUAGE_AV 20 // Arduino Nano se trouvant sur le PCB traquage_AV (ici c'est l'angle du servo qui est transmit)

#define CONTACT 2  //  PCB HMI, Nano se trouvant à gauche lorsqu'on regarde le U depuis sa base. Il gère les plaque de contact et les LED

#define ADRESSE_ROUE 19// PCB Puissance, "Arduino 2" Nano

#define TRAQUAGE_AR // Arduino Nano se trouvant sur le PCB traquage_AR (pcb du haut de la tête) il gère la fumée

#define SON // PCB HMI,  Nano se trouvant à droite lorsqu'on regarde le U depuis sa base. Il gère le HP

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

byte dataBuffer[BUFFER_SIZE];
byte output = 0;
//Il stoque la valeure actuelle du flanc montant pour les boutons dont la correspondance est telle:
//0: null; 1: A; 2: B, 3: NORTH, 4: SOUTH
bool flancsMontants[] = {false, false, false, false, false};
byte vie = 3;// Variable qui indique le nombre de vie restante
int message;//adresse du capteur qui lui parle
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

int stateMenuPos = 0; // Position du curseur

void setup() {
  Wire.begin(ADRESSE_INTELLIGENCE_CENTRALE);
  Wire.onReceive(receiveEvent);
  Serial2.begin(115200);
  Serial.begin(115200);
  Serial.println("Setup completed");
}

void loop() {
  communicationManette(); // On commence par communiquer les dernières infos avec la manette

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

/*
   Fonction qui gère le mode automatique des robots
   Pour le moment elle récupère uniquement l'angle du servo et le transmet aux roues.
*/
void loopAutomatique() {
  if (checkPause()) { // quitte directement la loop si la pause est pressée et évite que le "state" puisse être changé dans la fonction
    return;
  }
  output = 4;
  Wire.beginTransmission(ADRESSE_ROUE);
  Wire.write(anglePixy);
}

/*
   Fonction qui gère le mode manuel des robots
   Elle transmet la position des joysticks a l'arduino des roues
*/
void loopManuel() {
  if (checkPause()) { // quitte directement la loop si la pause est pressée et évite que le "state" puisse être changé dans la fonction
    return;
  }
  output = 27;
  return;
  Wire.beginTransmission(ADRESSE_ROUE);
  Wire.write(ADRESSE_ROUE);
  if (AxisLX() >= 134 || AxisLX() <= 120) {
    Wire.write(AxisLX());
  }
  else {
    Wire.write(127);
  }
  if (AxisLY() >= 134 || AxisLY() <= 120) {
    Wire.write(AxisLY());
  }
  else {
    Wire.write(127);
  }
  Wire.endTransmission();
}

/*
   Fonction qui gère la réception des messages sur le bus I2C central

*/
void receiveEvent(int howMany) {
  message = (uint8_t)Wire.read();
  if (message == TRAQUAGE_AV)
  {
    anglePixy = (uint8_t)Wire.read();//récupère l'angle reçu
  }
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
    temp = true;
  }
  flancsMontants[flancId] = button;
  return temp;
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
  while (Serial2.available() < 8) { // controlle la longueure de la tramme et si elle ne correspond pas il quitte et remet à zero les buffers de boutons
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
  if (currentState == state)return currentState; // Evite de traiter inutilement les données s'il n'y a pas de changement
  //previousState = currentState; // non utilisé car remplacé par le savedMode
  currentState = state;
  return currentState;
}
/*
   @func bool checkPause Vérifie s'il y a une demande de pause lors de la partie
   @param null
   @return bool
*/
bool checkPause() {
  if (ButtonFlanc(ButtonB(), 2)) {
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
