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
bool flancsMontants[] = {false, false, false, false, false}; //0: null; 1: A; 2: B, 3: NORTH, 4: SOUTH
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
  Serial.begin(115200, SERIAL_8N1);
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
   Vérifie s'il y a une demande de pause lors de la partie
   Retourne vrai si il y a demande de pause
   Retourne faux si pas demande de pause
*/
bool checkPause() {
  if (ButtonFlanc(ButtonB(), 2)) {
    setState(State::PauseGenerale, 0);
    return true;
  }
  return false;
}

/*
   Fonction qui gère le mode automatique des robots
   Pour le moment elle récupère uniquement l'angle du servo et le transmet aux roues.
*/
void loopAutomatique() {
  if (checkPause()) { // quitte directement la loop si la pause est pressée et évite que le "state" puisse être changé dans la fonction
    return;
  }
  Wire.write(ADRESSE_ROUE);
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
  Wire.write(ADRESSE_ROUE);
  if (AxisLX() >= 132 || AxisLX <= 122) {
    Wire.write(AxisLX());
  }
  else {
    Wire.write(127);
  }
  if (AxisLY() >= 132 || AxisLY <= 122) {
    Wire.write(AxisLY());
  }
  else {
    Wire.write(127);
  }
}
/*
   Change la position du curseur dans les menus à l'aide des touches NORTH et SOUTH
   Reçoit la taille du menu
   Retourne la position du curseur (un nombre entre 0 et la taille du menu)
*/
int changeCursorPosition(int sizeMenu) {

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

void loopPauseGenerale() {
  int tailleMenu = 2;
  int pos = changeCursorPosition(tailleMenu);//changement position curseur

  switch (pos) {
    case 0:
      output = 12; // Info d'affichage pour la manette

      if (ButtonFlanc(ButtonA(), 1)) {
        setState(State::MenuGO);
      }
      break;
    case 1:
      output = 18; // Info d'affichage pour la manette

      if (ButtonFlanc(ButtonA(), 1)) {
        setState(State::MenuSelection, 0);
      }
      break;
  }
}

/*
   Gère le menu général de selection de mode (affichage 1 et boutons)

*/
void  loopMenuSelection() {
  int tailleMenu = 2;
  int pos = changeCursorPosition(tailleMenu);//changement position curseur
  State selectedMode;

  switch (pos) {
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

/*
   Gère le menu GO qui marque une pause avant de lancer la partie (affichage 1 et 2 et boutons)
*/
void  loopMenuGo() {
  output = 3;
  if (ButtonFlanc(ButtonA(), 1)) {
    setState(savedMode);
  }
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
    Les fonctions suivantes permettent de récupèrer l'état de n'importe quel bouton/joystick plus loin dans le code
*/
byte AxisLX()       {
  return dataBuffer[2];
}
byte AxisLY()       {
  return dataBuffer[3];
}
byte AxisRX()       {
  return dataBuffer[4];
}
byte AxisRY()       {
  return dataBuffer[5];
}
byte AxisLT()       {
  return dataBuffer[6];
}
byte AxisRT()       {
  return dataBuffer[7];
}

bool ButtonA()      {
  return bitRead(dataBuffer[0], A);
}
bool ButtonB()      {
  return bitRead(dataBuffer[0], B);
}
bool ButtonX()      {
  return bitRead(dataBuffer[0], X);
}
bool ButtonY()      {
  return bitRead(dataBuffer[0], Y);
}
bool ButtonWEST()   {
  return bitRead(dataBuffer[1], WEST);
}
bool ButtonEAST()   {
  return bitRead(dataBuffer[1], EAST);
}
bool ButtonNORTH()  {
  return bitRead(dataBuffer[1], NORTH);
}
bool ButtonSOUTH()  {
  return bitRead(dataBuffer[1], SOUTH);
}
bool ButtonLB()     {
  return bitRead(dataBuffer[0], LB);
}
bool ButtonRB()     {
  return bitRead(dataBuffer[0], RB);
}
bool ButtonLS()     {
  return bitRead(dataBuffer[0], LS);
}
bool ButtonRS()     {
  return bitRead(dataBuffer[0], RS);
}
bool ButtonSTART()  {
  return bitRead(dataBuffer[0], START);
}
bool ButtonSELECT() {
  return bitRead(dataBuffer[0], SELECT);
}
/*
   Permet de détecter les flancs montants des boutons dans une seule fonction
   Reçoit le nom du bouton ainsi que son numéro d'identification de flanc (les numéros sont déclarés dans un booléen au début du code)
   Retourne vrai si c'est un flanc montant (nouvel état du bouton)
   Retourne faux si l'état est le même que précédemment
*/
bool ButtonFlanc(bool button, int flancId) {
  if (button && !flancsMontants[flancId]) {
    return true;
    flancsMontants[flancId] = true;
  }
  else if (!button) {
    flancsMontants[flancId] = false;
  }
  return false;
}

/*
   Gère la communication avec l'esp32 du groupe manette installé sur le pcb. On commence par réenvoyer les données que l'on possède ce qui fait que l'esp32 nous envoie
   les siennes directement.
*/
void communicationManette() {
  uint8_t dataBufferWrite[2] = {output, sonEtVibreur};// réenvoie les données à la manette
  Serial.write(dataBufferWrite, 2);
  while (Serial.available() < 8) { // controlle la longueure de la tramme et si elle ne correspond pas il quitte et remet à zero les buffers de boutons
  }
  Serial.readBytes(dataBuffer, BUFFER_SIZE); //lit les infos en provenance de la manette
}

/*
   Elle change l'état actuelle de la variable state et retourne son état actuel.
   Permet de faire d'autres actions sur des variables lors d'un changement d'état directement dans cette fonction si nécessaire
   Evite de passer par une variable grobale.
   Reçoit l'argument facultatif menuPos
   Retourne la variable currentState
*/
State setState(State state, int menuPos = -1) {
  if (menuPos > -1) {
    stateMenuPos = menuPos;
  }
  if (currentState == state)return currentState; // Evite de traiter inutilement les données s'il n'y a pas de changement
  //previousState = currentState; // non utilisé car remplacé par le savedMode
  currentState = state;
  return currentState;
}
