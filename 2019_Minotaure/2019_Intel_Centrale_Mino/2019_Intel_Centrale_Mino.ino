// Code de l'Arduino Mega gérant l'intelligence centrale du Minotaure
//Son but est de récolter toutes les informations des capteurs et de prendre une décision en conséquence
// Dany VALADO Lucien PRUVOT Paul THONNEY
// 28.05.19


#include <Wire.h> //I2C

//toutes les adresses I2C

#define INTEL 1 // adresse de l'intelligence centrale, arduino nano sur le PCB Bluetooth

#define TETE // Arduino Nano se trouvant sur le PCB traquage_AV (pcb du bas de la tête)

#define CONTACT 2  //  PCB HMI, Nano se trouvant à gauche lorsqu'on regarde le U depuis sa base. Il gère les plaque de contact et les LED

#define ROUES // PCB Puissance, "Arduino 2" Nano

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

typedef enum State {
  Automatique,
  Manuel,
  PauseGenerale,
  MenuSelection,
  MenuGO,
} State;

State currentState = State::MenuSelection;

byte vie = 3;

int message;//adresse du capteur qui lui parle
int angle;
int angleMoteur;

//int objet = 0;//nombre d'objet capté par la Pixy
//int distance_gauche, distance_droite;//distance des lasers
//int distance_avant, distance_arriere;//distance des ultrasons
//long  temps1 = 0;
//int x = 0; //position de joystick virtuel représentant la direction du déplacement
//int y = 1024;
//byte xx [2];//position à envoyer par I2C au nano puissance
//byte yy [2];
//int angle;//angle de la direction du déplacement, comme x et y

void setup()
{
  Wire.begin(INTEL);
  Wire.onReceive(receiveEvent);
  Serial.begin(115200, SERIAL_8N1);
}


void loop()
{
  communicationManette();
  switch (currentState) {
    case State::Automatique: {

        loopAutomatique();

        break;
      }
    case State::Manuel: {
        loopManuel();

        break;
      }
  }
}

/* Elle change l'état actuelle de la variable state et retourne son état actuel
    Permet de faire d'autres actions sur des variables lors d'un changement d'état directement dans cette fonction si nécessaire
*/
State setState(State state) {
  currentState = state;
  return currentState;
}

void communicationManette() {
  uint8_t dataBufferWrite[2] = {127, yolo};// réenvoie les données à la manette
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
 *  Les fonctions suivantes permettent de récupèrer l'état de n'importe quel bouton/joystick plus loin dans le code
 */
byte AxisLX(){
  return dataBuffer[2];
}

byte AxisLY(){
  return dataBuffer[3];
}

byte AxisRX(){
  return dataBuffer[4];
}

byte AxisRY(){
  return dataBuffer[5];
}

byte AxisLT(){
  return dataBuffer[6];
}

byte AxisRT(){
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

/*
 * Fonction qui gère le mode automatique des robots
 */
void loopAutomatique() {

}

/*
 * Fonction qui gère le mode manuel des robots
 */
void loopManuel(){
  
}

/*
 * Fonction qui gère la réception des messages sur le bus I2C central
 */
void receiveEvent(int howMany){
  message = (uint8_t)Wire.read();

  if (message == TRAQUAGE_AV)
  {
    angle = (uint8_t)Wire.read();//récupère l'angle reçu
  }
}
