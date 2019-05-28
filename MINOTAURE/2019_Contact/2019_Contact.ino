// Code de l'Arduino Nano gérant les plaques de contacts
// Dany VALADO
// 01.07.2017
// Ce qu'il reste à ajouter dans le code :
//  - mode pause et mode combat
//  - la perte de point de vie
//  - l'animation de la mort du Minotaure
//  - l'identification visuelle de la plaque touchée par le Chevalier
//  - l'animation d'un mode spéciale comme un boost/ralentissement de vitesse, une paralysie/aveuglement/étourdissement dû à l'attaque spéciale du chevalier

#include <Adafruit_NeoPixel.h> //LED RGB
#include <Wire.h> //I2C


// Adresses I2C, il n'y a que l'intelligence centrale, car c'est le seul micro avec lequel celui-la est sensé communiquer. Dans le code 2018, il envoyait lui-même l'info au micro du son pour qu'il fasse un son de coup,
//mais dans un souci de centralisation (imposée par M. Locatelli),c'est l'intel centrale qui relaie toute information. 

#define INTEL 1  // adresse de l'intelligence centrale, arduino nano sur le PCB Bluetooth

#define CONTACT 2 //adresse de cet arduino


//codes pour actions

#define COUP 10

#define OBJET 12




#define CONTACT0_PIN   2 //Pins des boutons
#define CONTACT1_PIN   3
#define CONTACT2_PIN   4
#define CONTACT3_PIN   5
#define CONTACT4_PIN   6
#define CONTACT5_PIN   8

#define MATRICE0_PIN    7 //pins des matrices de LEDs
#define MATRICE1_PIN    9
#define MATRICE2_PIN    10
#define MATRICE3_PIN    11
#define MATRICE4_PIN    12
#define MATRICE5_PIN    13

#define PIXEL_NOMBRE 10 //nombre de LED sur une plaque
#define MATRICE_NOMBRE 6 //nombre de plaques de contacts
#define LUMINOSITE 50 //luminosité des LEDs
#define TEMPS_CLIGNETEMENT 100 

Adafruit_NeoPixel matrice[MATRICE_NOMBRE] = {Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE0_PIN, NEO_GRB + NEO_KHZ800),//déclaration des noms de chaque matrice sous forme de tableau
                                             Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE1_PIN, NEO_GRB + NEO_KHZ800),//possiblité d'utilisé un pointeur au lieu d'un tableau, voir programme Test_plaques_de_contact_pointeur
                                             Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE2_PIN, NEO_GRB + NEO_KHZ800),
                                             Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE3_PIN, NEO_GRB + NEO_KHZ800),
                                             Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE4_PIN, NEO_GRB + NEO_KHZ800),
                                             Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE5_PIN, NEO_GRB + NEO_KHZ800)
                                            };

byte oldContact[MATRICE_NOMBRE] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH}; //état du bouton de la boucle précèdente

uint64_t tempsActuel[MATRICE_NOMBRE] = {0, 0, 0, 0, 0, 0};
uint16_t cycle[MATRICE_NOMBRE] = {0, 0, 0, 0, 0, 0};

void setup()
{
  pinMode(CONTACT0_PIN, INPUT);
  pinMode(CONTACT1_PIN, INPUT);
  pinMode(CONTACT2_PIN, INPUT);
  pinMode(CONTACT3_PIN, INPUT);
  pinMode(CONTACT4_PIN, INPUT);
  pinMode(CONTACT5_PIN, INPUT);

  for (uint8_t i = 0; i < MATRICE_NOMBRE; i++) //configure chaque matrice de LED
  {
    matrice[i].begin();
    matrice[i].show();
    matrice[i].setBrightness(LUMINOSITE);
  }

  Wire.begin(ADRESSE_CONTACT);
  Serial.begin(9600);
}

void loop()
{
  int16_t newContact[MATRICE_NOMBRE] = { digitalRead(CONTACT0_PIN), //sauvegarde l'état du bouton
                                         // digitalRead(CONTACT1_PIN), //pas utilisé sur minotaure
                                         digitalRead(CONTACT2_PIN),
                                         digitalRead(CONTACT3_PIN),
                                         // digitalRead(CONTACT4_PIN),
                                         digitalRead(CONTACT5_PIN) //pas utilisé sur minotaure
                                       };

  for (uint8_t i = 0; i < MATRICE_NOMBRE; i++) //vérifie si le bouton NC est appuyé pour chaque matrice
  {
    if (newContact[i] == LOW && oldContact[i] == HIGH && millis() > 1000) // Détecte si on appuie sur un bouton NC aprés 1s
    {
      //Serial.println(i);
      Wire.beginTransmission(INTEL); // intilligence centrale
      Wire.write(COUP);//informe qu'il y a contact
      Wire.endTransmission();

      uint32_t rouge = matrice[i].Color(255, 0, 0);
      clignoteTousLed(rouge);
      Serial.println("contact");
    }
    oldContact[i] = newContact[i];
  }

  if ((unsigned long)(millis() - tempsActuel[0]) >= 100) //lumière arc-en-ciel avec un temps différent pour chaque matrice pour le fun
  {
    tempsActuel[0] = millis();
    arcenciel(0);
  }
  if ((unsigned long)(millis() - tempsActuel[2]) >= 150)
  {
    tempsActuel[2] = millis();
    arcenciel(2);
  }
  if ((unsigned long)(millis() - tempsActuel[3]) >= 200)
  {
    tempsActuel[3] = millis();
    arcenciel(3);
  }
  if ((unsigned long)(millis() - tempsActuel[5]) >= 250)
  {
    tempsActuel[5] = millis();
    arcenciel(5);
  }
}

void allumeLed(uint8_t n, uint32_t couleur) // n est le numero de la matrice, ici 1 à 6
{
  for (uint8_t i = 0; i < PIXEL_NOMBRE; i++) //allume LED par LED
    matrice[n].setPixelColor(i, couleur);
  matrice[n].show();
}

void clignoteLed(uint8_t n, uint32_t couleur)
{
  uint32_t invisible = matrice[n].Color(0, 0, 0);
  for (uint8_t i = 0; i < 5; i++)
  {
    //if (millis() % 1000 <= 500)// sans delay mais je ne veux pas que le mino perde un autre point de vie tout de suite après sinon c'est la mitraillette et le combat se fini d'un coup 
    allumeLed(n, couleur);
    delay(TEMPS_CLIGNETEMENT); // oui, je sais, j'ai mis des delay dégueulasses mais c'est dans mon cas, c'est facile et c'est pas grave
    //else
    allumeLed(n, invisible);
    delay(TEMPS_CLIGNETEMENT);
  }
}

void clignoteTousLed(uint32_t couleur)
{
  uint32_t invisible = matrice[0].Color(0, 0, 0);
  for (uint8_t i = 0; i < 4; i++)
  {
    for (uint8_t j = 0; j < 6; j++)
      allumeLed(j, couleur);
    delay(TEMPS_CLIGNETEMENT);
    for (uint8_t j = 0; j < 6; j++)
      allumeLed(j, invisible);
    delay(TEMPS_CLIGNETEMENT);
  }
}

void arcenciel(uint8_t n) //programme pris de la bibliothèque
{
  for (uint16_t i = 0; i < matrice[n].numPixels(); i++)
    matrice[n].setPixelColor(i, roue(n, (i + cycle[n]) & 255));
  matrice[n].show();
  cycle[n]++;
  if (cycle[n] >= 256)
    cycle[n] = 0;
}


uint32_t roue(uint8_t n, byte rouePos)//programme pris de la bibliothèque
{
  rouePos = 255 - rouePos;
  if (rouePos < 85) {
    return matrice[n].Color(255 - rouePos * 3, 0, rouePos * 3);
  }
  if (rouePos < 170) {
    rouePos -= 85;
    return matrice[n].Color(0, rouePos * 3, 255 - rouePos * 3);
  }
  rouePos -= 170;
  return matrice[n].Color(rouePos * 3, 255 - rouePos * 3, 0);
}
