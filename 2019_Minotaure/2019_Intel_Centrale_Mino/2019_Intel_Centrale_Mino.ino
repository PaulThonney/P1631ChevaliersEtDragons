// Code de l'Arduino Mega gérant l'intilligence centrale du Minotaure
//Son but est de récolter toutes les informations des capteurs et de prendre une décision en conséquence
// Dany VALADO
// 01.07.2017
// Ce qu'il reste à ajouter dans le code :
//  - beaucoup de chose, vraiment beaucoup

#include <Wire.h> //I2C

//toutes les adresses I2C

#define INTEL 1 // adresse de l'intelligence centrale, arduino nano sur le PCB Bluetooth

#define TETE // Arduino Nano se trouvant sur le PCB traquage_AV (pcb du bas de la tête)

#define CONTACT 2  //  PCB HMI, Nano se trouvant à gauche lorsqu'on regarde le U depuis sa base. Il gère les plaque de contact et les LED

#define ROUES // PCB Puissance, "Arduino 2" Nano

#define TRAQUAGE_AR // Arduino Nano se trouvant sur le PCB traquage_AR (pcb du haut de la tête) il gère la fumée

#define SON // PCB HMI,  Nano se trouvant à droite lorsqu'on regarde le U depuis sa base. Il gère le HP


//codes pour actions

#define COUP

#define OBJET

int message;//adresse du capteur qui lui parle

byte vie = 3;

int angle;

int angleMoteur;


void setup()
{
  Wire.begin(INTEL);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
}

void loop()
{
  Check Buttons;
  
}

void receiveEvent(int howMany)
{
  message = (uint8_t)Wire.read(); 
  
  if (message == TRAQUAGE_AV)
  {
   angle = (uint8_t)Wire.read();//récupère l'angle reçu
  }

}

    
