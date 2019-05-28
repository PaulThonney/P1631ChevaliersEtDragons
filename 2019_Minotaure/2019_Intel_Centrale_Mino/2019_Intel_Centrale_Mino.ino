/* Code de l'Arduino gérant l'intelligence centrale du Minotaure
  Son but est de récolter toutes les informations des capteurs et de prendre des décisions en conséquence
  AUTEURS: Dany VALADO (2018) Lucien PRUVOT Paul THONNEY
  DATE: 28.05.19
  REMERCIEMENTS: Merci à Maxime SCHARWATH et Joan MAILLARD pour leur aide
*/

#include <Wire.h> //I2C

//toutes les adresses I2C

#define INTEL 1 // adresse de l'intelligence centrale, arduino nano sur le PCB Bluetooth

#define TRAQUAGE_AV 20 // Arduino Nano se trouvant sur le PCB traquage_AV (pcb du bas de la tête)

#define CONTACT 2  //  PCB HMI, Nano se trouvant à gauche lorsqu'on regarde le U depuis sa base. Il gère les plaque de contact et les LED

#define ROUES 19// PCB Puissance, "Arduino 2" Nano

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
bool flancsMontants[] = {false, false, false}; //0: SOUTH & NORTH; 1: A; 2: NULL
byte vie = 3;// Variable qui indique le nombre de vie restante
int message;//adresse du capteur qui lui parle
byte anglePixy;
byte sonEtVibreur; //4 premiers bits: buzzer; 4 derniers bits: vibreur

typedef enum State { // On définit les états possible de la machine
  Automatique,
  Manuel,
  PauseGenerale1,
  PauseGenerale2,
  MenuSelection1,
  MenuSelection2,
  MenuGO,
} State;


State savedMode = State::Automatique;
State currentState = State::MenuSelection1; // On démarre sur le menu de sélection
State previousState; // Ancien état

void setup()
{
  Wire.begin(INTEL);
  Wire.onReceive(receiveEvent);
  Serial.begin(115200, SERIAL_8N1);
}

void loop()
{
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
    case State::PauseGenerale1: { // Mode pause avec curseur sur "Reprendre"

        loopPauseGenerale1();

        break;
      }
    case State::PauseGenerale2: { // Mode pause avec curseur sur "Menu Principal"

        loopPauseGenerale2();

        break;
      }
    case State::MenuSelection1: { // Mode menu principal avec curseur sur "Automatique"

        loopMenuSelection1();

        break;
      }

    case State::MenuSelection2: { // Mode menu principal avec curseur sur "Manuel"

        loopMenuSelection2();

        break;
      }
    case State::MenuGO: { // Mode MenuGo avec affichage d'un logo "GO"

        loopMenuGo();

        break;
      }
  }
}

/*
   Fonction qui gère le mode automatique des robots
*/
void loopAutomatique() {

}

/*
   Fonction qui gère le mode manuel des robots
*/
void loopManuel() {

}

void loopPauseGenerale1() {
  output = 29;
  if (ButtonNORTH() && !flancsMontants[0] || ButtonSOUTH() && !flancsMontants[0]) {
    setState(State::PauseGenerale2);
    flancsMontants[0] = true;
  }
  else if (!ButtonNORTH() && !ButtonSOUTH()) {
    flancsMontants[0] = false;
  }
  if (ButtonA() && !flancsMontants[1]) {
    setState(savedMode);
    flancsMontants[1] = true;
  }
  else if (!ButtonA()) {
    flancsMontants[1] = false;
  }
}

void loopPauseGenerale2() {
  output = 30;
  if (ButtonNORTH() && !flancsMontants[0] || ButtonSOUTH() && !flancsMontants[0]) {
    setState(State::PauseGenerale1);
    flancsMontants[0] = true;
  }
  else if (!ButtonNORTH() && !ButtonSOUTH()) {
    flancsMontants[0] = false;
  }
  if (ButtonA() && !flancsMontants[1]) {
    setState(State::MenuSelection1);
    flancsMontants[1] = true;
  }
  else if (!ButtonA()) {
    flancsMontants[1] = false;
  }
}
void  loopMenuSelection1() {
  output = 1;
  if (ButtonNORTH() && !flancsMontants[0] || ButtonSOUTH() && !flancsMontants[0]) {
    setState(State::MenuSelection2);
    flancsMontants[0] = true;
  }
  else if (!ButtonNORTH() && !ButtonSOUTH()) {
    flancsMontants[0] = false;
  }
  if (ButtonA() && !flancsMontants[1]) {
    savedMode = State::Automatique;
    setState(State::MenuGO);
    flancsMontants[1] = true;
  }
  else if (!ButtonA()) {
    flancsMontants[1] = false;
  }
}
void  loopMenuSelection2() {
  output = 2;
  if (ButtonNORTH() && !flancsMontants[0] || ButtonSOUTH() && !flancsMontants[0]) {
    setState(State::MenuSelection1);
    flancsMontants[0] = true;
  }
  else if (!ButtonNORTH() && !ButtonSOUTH()) {
    flancsMontants[0] = false;
  }
  if (ButtonA() && !flancsMontants[1]) {
    savedMode = State::Manuel;
    setState(State::MenuGO);
    flancsMontants[1] = true;
  }
  else if (!ButtonA()) {
    flancsMontants[1] = false;
  }
}

void  loopMenuGo() {
  if(millis()%2000>1000){
    output = 30;// Image 1
  }else{
    output = 30;// Image 2
  }
  
  if (ButtonA() && !flancsMontants[1]) {
    setState(savedMode);
    flancsMontants[1] = true;
  }
  else if (!ButtonA()) {
    flancsMontants[1] = false;
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
byte AxisLX() {
  return dataBuffer[2];

}

byte AxisLY() {
  return dataBuffer[3];
}

byte AxisRX() {
  return dataBuffer[4];
}

byte AxisRY() {
  return dataBuffer[5];
}

byte AxisLT() {
  return dataBuffer[6];
}

byte AxisRT() {
  return dataBuffer[7];
}

bool ButtonA() {
  return bitRead(dataBuffer[0], A);
}

bool ButtonB() {
  return bitRead(dataBuffer[0], B);
}

bool ButtonX() {
  return bitRead(dataBuffer[0], X);
}

bool ButtonY() {
  return bitRead(dataBuffer[0], Y);
}

bool ButtonWEST() {
  return bitRead(dataBuffer[1], WEST);
}

bool ButtonEAST() {
  return bitRead(dataBuffer[1], EAST);
}

bool ButtonNORTH() {
  return bitRead(dataBuffer[1], NORTH);
}

bool ButtonSOUTH() {
  return bitRead(dataBuffer[1], SOUTH);
}

bool ButtonLB() {
  return bitRead(dataBuffer[0], LB);
}

bool ButtonRB() {
  return bitRead(dataBuffer[0], RB);
}

bool ButtonLS() {
  return bitRead(dataBuffer[0], LS);
}

bool ButtonRS() {
  return bitRead(dataBuffer[0], RS);
}

bool ButtonSTART() {
  return bitRead(dataBuffer[0], START);
}

bool ButtonSELECT() {

  return bitRead(dataBuffer[0], SELECT);
}

void communicationManette() {
  uint8_t dataBufferWrite[2] = {output, sonEtVibreur};// réenvoie les données à la manette
  Serial.write(dataBufferWrite, 2);
  if (Serial.available() < 8) { // controlle la longueure de la tramme et si elle ne correspond pas il quitte et remet à zero les buffers de boutons
    for (int i = 0; i < BUFFER_SIZE; i++) {
      dataBuffer[i] = 0;
    }
    return;
  }
  Serial.readBytes(dataBuffer, BUFFER_SIZE); //lit les infos en provenance de la manette
}

/*
    Elle change l'état actuelle de la variable state et retourne son état actuel
    Permet de faire d'autres actions sur des variables lors d'un changement d'état directement dans cette fonction si nécessaire
*/
State setState(State state) {
  if (currentState == state)return currentState;
  previousState = currentState;
  currentState = state;
  return currentState;
}
