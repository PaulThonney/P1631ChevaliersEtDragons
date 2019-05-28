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

typedef enum State {
  Automatique,
  Manuel,
  PauseGenerale,
  MenuSelection
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
  Serial.begin(9600);
}

void loop()
{
checkButtons();
  switch (currentState) {
    case State::Automatique: {
      
        loopAutomatique();
        
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

void checkButtons(){
  
}

void loopAutomatique(){
  
}

void receiveEvent(int howMany)
{
  message = (uint8_t)Wire.read();

  if (message == TRAQUAGE_AV)
  {
    angle = (uint8_t)Wire.read();//récupère l'angle reçu
  }

}
