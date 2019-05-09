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



int message;//adresse du capteur qui lui parle
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
  if (adresse_capteur == ADRESSE_PIXY1)//il ne fait rien
  {
    Serial.println(objet);
    adresse_capteur = 0; //pour éviter qu'il ne reste coincé ici car si pas de communication adresse_capteur ne remet pas à 0 avec receivevent
  }
  else if (adresse_capteur == ADRESSE_CONTACT)//il ne fait rien
  {
    Serial.println("Contact");
    adresse_capteur = 0; // remise à 0 de la variable
  }
  else if (adresse_capteur == ADRESSE_COULEUR)
  {
    Serial.println("Couleur!");

    Wire.beginTransmission(ADRESSE_ROUE);//informe au nano puissance qu'il doit reculer
    Wire.write(ADRESSE_COULEUR);
    Wire.endTransmission();

    adresse_capteur = 0; // remise à 0 de la variable
  }
  else if (adresse_capteur == ADRESSE_SERVO)
  {
    int angleMoteur = 180 - angle; //inverse l'angle car pas le même référence

    Wire.beginTransmission(ADRESSE_ROUE);
    Wire.write(ADRESSE_SERVO);
    Wire.write(angleMoteur);//donne l'angle de la direction à prendre
    Wire.endTransmission();

    adresse_capteur = 0; // remise à 0 de la variable
  }
  else
    Serial.println("rien");
}

void receiveEvent(int howMany)
{
  //Serial.println(howMany);
  message = Wire.read(); //récuère l'addresse de l'émetteur
  if (message == )
  {
    objet = Wire.read();//récupère le nomre d'objet
    //Serial.println("PIXY");
  }
  else if (adresse_capteur == ADRESSE_CONTACT)
    Serial.println("Contact");//il ne fait rien
  else if (adresse_capteur == ADRESSE_SERVO)
    angle = (uint8_t)Wire.read();//récupère l'angle reçu
}
