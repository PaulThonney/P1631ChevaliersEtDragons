#include <Wire.h>

void setup()
{
Wire.begin(); // Rejoindre le bus I2C (Pas besoin d adresse pour le maitre)
}

void loop()
{
//contenu du programme
Wire.beginTransmission(0x01); // Envoyer vers device #4
Wire.write("a"); // Envoi un 1
Wire.endTransmission(); // Arreter la transmission
delay(4000); // Attendre 1s
Wire.beginTransmission(0x01); // Envoyer vers device #4
Wire.write("b"); // Envoi un 0
Wire.endTransmission(); // Arreter la transmission
delay(4000); // Attendre 2s
} 
