// Code de l'Arduino Mega gérant l'intilligence centrale du Minotaure
//Son but est de récolter toutes les informations des capteurs et de prendre une décision en conséquence
// Dany VALADO
// 01.07.2017
// Ce qu'il reste à ajouter dans le code :
//  - beaucoup de chose, vraiment beaucoup

#include <Wire.h> //I2C

//toutes les adresses I2C

#define INTEL  // adresse de l'intelligence centrale, arduino nano sur le PCB Bluetooth

#define TRAQUAGE_AV // Arduino Nano se trouvant sur le PCB traquage_AV (pcb du bas de la tête)

#define CONTACT  //  PCB HMI, Nano se trouvant à gauche lorsqu'on regarde le U depuis sa base. Il gère les plaque de contact et les LED

#define ROUES // PCB Puissance, "Arduino 2" Nano

#define TRAQUAGE_AR // Arduino Nano se trouvant sur le PCB traquage_AR (pcb du haut de la tête) il gère la fumée

#define SON // PCB HMI,  Nano se trouvant à droite lorsqu'on regarde le U depuis sa base. Il gère le HP


//codes pour actions

#define COUP



int message;//adresse du capteur qui lui parle

byte vie = 3;

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
  if (message == CONTACT)//il ne fait rien
  {
    Vie-- ;

    Wire.beginTransmission(SON);
    Wire.write(COUP);    //donne l'angle de la direction à prendre
    Wire.endTransmission();
    
    message = 0; //pour éviter qu'il ne reste coincé ici car si pas de communication message ne remet pas à 0 avec receivevent
  }
  else if (message == TRAQUAGE_AV)
  {
    int angleMoteur = 180 - angle;    //inverse l'angle car pas le même référence

    Wire.beginTransmission(ROUES);
    Wire.write(angleMoteur);    //donne l'angle de la direction à prendre
    Wire.endTransmission();

    message = 0;    // remise à 0 de la variable
  }
 
}

void receiveEvent(int howMany)
{
  message = Wire.read(); //récuère l'addresse de l'émetteur
 
  if (message == TRAQUAGE_AV)
  {
   angle = (uint8_t)Wire.read();//récupère l'angle reçu
  }

}

    
