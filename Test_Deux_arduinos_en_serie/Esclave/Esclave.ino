#include <Wire.h> // Librairie pour la communication I2C

const int L1 = 2; // broche 2 du micro-contrôleur se nomme maintenant : L1

long int x;

void setup()
{
  //NOTE: Le 0x04 est TRES IMPORTANT pour le fonctionnement du programme
  Wire.begin(0x01); // Rejoindre le bus à l'adresse #4
  Wire.onReceive(receiveEvent); // Preparer une fonction spécifique a la reception de donnee
  Serial.begin(9600); // Demarrer la liaison serie avec le PC
  pinMode(L1, OUTPUT); // L1 est une broche de sortie


  digitalWrite (L1, HIGH);
}


void loop()
{
//  Wire.beginTransmission(0x01); // Envoyer vers device #4
//  Wire.write(1); // Envoi un 1
//  Wire.endTransmission(); // Arreter la transmission
//  delay(4000); // Attendre 1s
//  Wire.beginTransmission(0x01); // Envoyer vers device #4
//  Wire.write(0); // Envoi un 0
//  Wire.endTransmission(); // Arreter la transmission
//  delay(4000); // Attendre 2s


}

// Fonction qui s execute si quelque chose est present sur l interface
void receiveEvent(int howMany)
{
  int x = Wire.read(); // recevoir un chiffre
  Serial.println(x); // afficher ce chiffre sur l'interface serie
  if (x == 1)
  {
    digitalWrite(L1, HIGH); // allumer L1
  }
  if (x == 0)
  {
    digitalWrite(L1, LOW); // eteindre L1
  }
}
