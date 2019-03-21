// Code de l'Arduino Nano gérant les plaques de contacts du chevalier, quasi identique à celui de 2017 donc dsl s'il manque des commentaires mais j'ai pas eu le temps de comprendre leur prog
// Dany VALADO
// 01.07.2017
// Ce qu'il reste à ajouter dans le code :
//  - mode pause et mode combat
//  - animation gain/perte point de vie
//  - l'animation de sa mort ou de sa victoire
//  - l'identification visuelle de la plaque touchée
//  - l'animation d'un mode spéciale comme un boost/ralentissement de vitesse, une paralysie/aveuglement/étourdissement
//  - mode attaque spéciale et mode bouclier

#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define ADRESSE_INTILLIGENCE_CENTRALE 8
#define ADRESSE_CONTACT 10
#define ADRESSE_SON 14

#define CONTACT 1

#define CONTACT0_PIN   2 //droite
#define CONTACT1_PIN   3 //arrière gauche
#define CONTACT2_PIN   4 //gauche 
#define CONTACT3_PIN   5 //arrière droite
#define CONTACT4_PIN   6 //avant gauche
#define CONTACT5_PIN   8 //avant droite

#define MATRICE0_PIN    7
#define MATRICE1_PIN    9
#define MATRICE2_PIN    10
#define MATRICE3_PIN    11
#define MATRICE4_PIN    12
#define MATRICE5_PIN    13

#define PIXEL_NOMBRE 10
#define MATRICE_NOMBRE 6
#define LUMINOSITE 100
#define TEMPS_CLIGNETEMENT 100

Adafruit_NeoPixel matrice[MATRICE_NOMBRE] = {Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE0_PIN, NEO_GRB + NEO_KHZ800),
                                             Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE1_PIN, NEO_GRB + NEO_KHZ800),
                                             Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE2_PIN, NEO_GRB + NEO_KHZ800),
                                             Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE3_PIN, NEO_GRB + NEO_KHZ800),
                                             Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE4_PIN, NEO_GRB + NEO_KHZ800),
                                             Adafruit_NeoPixel(PIXEL_NOMBRE, MATRICE5_PIN, NEO_GRB + NEO_KHZ800)
                                            };

byte oldContact[MATRICE_NOMBRE] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

uint16_t r[MATRICE_NOMBRE] = {255, 255, 255, 255, 255, 255};
uint16_t g[MATRICE_NOMBRE] = {0, 0, 0, 0, 0, 0};
uint16_t b[MATRICE_NOMBRE] = {0, 0, 0, 0, 0, 0};

uint64_t tempsActuel[MATRICE_NOMBRE] = {0, 0, 0, 0, 0, 0};
uint16_t cycle[MATRICE_NOMBRE] = {0, 0, 0, 0, 0, 0};

int contact = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long currentTime = 0;
int gotHit = 0;
int invincible = 0;
byte shield = 9;
int x = 0;
int lifeDelta = 0; //1 => Bouclier activé, touché sur le bouclier => +1 point de vie ; 0 => Pas touché ; 2 => touché, -1 PV
int previousLifeDelta = 0;
int nbLed = 0;
int nbLedShield = 1;
int shieldState = 0; //0 => bouclier pas activé; 1 => bouclier activé
int state = 1;


void receiveEvent(int howMany)
{
  // state = Wire.read();

  shieldState = Wire.read();

  //Serial.print("State : ");
  //Serial.print(state);
}

  void requestEvent(int howMany) {

    Serial.println("request Event");
    /*if (shieldState == 1 && contact == 2) {*/
    Wire.write(lifeDelta);/*
  }
  else Wire.write(contactState); */
    Serial.println(lifeDelta);
  }



void checkContacts()
{
  int16_t newContact[MATRICE_NOMBRE] = { digitalRead(CONTACT0_PIN),
                                         digitalRead(CONTACT1_PIN), //pas utilisé sur minotaure
                                         digitalRead(CONTACT2_PIN),
                                         digitalRead(CONTACT3_PIN),
                                         digitalRead(CONTACT4_PIN),
                                         digitalRead(CONTACT5_PIN) //pas utilisé sur minotaure
                                       };
  if (lifeDelta == 0)
  {
    for (uint8_t i = 0; i < MATRICE_NOMBRE; i++)
    {
      if (newContact[i] == LOW && oldContact[i] == HIGH && millis() > 1000) // Détecte si on appuie sur un bouton NC aprés 1s
      {
        contact = 1;  //Plaque de contact sans bouclier
        Wire.beginTransmission(ADRESSE_SON);
        Wire.write(random(0, 21));
        Wire.endTransmission();

        uint32_t rouge = matrice[i].Color(255, 0, 0);
        clignoteTousLed(rouge);
        Serial.println("contact");
        nbLed = i;
      }
    }
    contact = 0;

    if (contact != 0 && shieldState == 0 && invincible == 0)
    {
      //Serial.print("got hit");
      gotHit = 1;
      lifeDelta = 2;
      resetTime();
    }
    if (shieldState == 1 && contact == 2)
    {
      lifeDelta = 1;
      //Serial.println("YAY 1HP IM TU GUD");
    }
  }
}

void shieldCheck()
{
  if (shieldState == 1)
  {
    invincible = 1;
    uint32_t bleu = matrice[0].Color(0, 0, 255);
    for (uint8_t i = 0; i < 5; i++)
      allumeLed(i, bleu);
  }
}


void invincibleCheck()
{
  invincible = 1;
  if (currentTime > 2000)
    gotHit = 0;
}


void resetTime()
{
  previousMillis = currentMillis;
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

void hitLed(int nbLed)
{
  uint32_t rouge = matrice[0].Color(255, 0, 0);
  clignoteTousLed(rouge);
}

void stopHitLed(int nbLed)
{

  if (shieldState == 0 || lifeDelta == 2 && shieldState == 1)
  {
    lifeDelta = 0;
  }
  //  digitalWrite(led[nbLed], LOW);
}

void loop()
{
  currentMillis = millis();
  currentTime = currentMillis - previousMillis;

  //Serial.println(currentTime);
  shieldCheck();

  if (gotHit == 1)
  {
    invincibleCheck();
    hitLed(nbLed);
  }
  else
  {
    stopHitLed(nbLed);
    checkContacts();
  }
  uint32_t bleu = matrice[4].Color(0, 0, 255);
  uint32_t rouge = matrice[5].Color(255, 0, 0);
  allumeLed(4, bleu);
  allumeLed(5, rouge);
  allumeLed(1, bleu);
  allumeLed(3, rouge);
}
  void allumeLed(uint8_t n, uint32_t couleur) // n est le numero de la matrice, ici 1 à 6
  {
    for (uint8_t i = 0; i < PIXEL_NOMBRE; i++)
      matrice[n].setPixelColor(i, couleur);
    matrice[n].show();
  }

  void clignoteLed(uint8_t n, uint32_t couleur)
  {
    uint32_t invisible = matrice[n].Color(0, 0, 0);
    for (uint8_t i = 0; i < 5; i++)
    {
      //if (millis() % 1000 <= 500)
      allumeLed(n, couleur);
      delay(TEMPS_CLIGNETEMENT); // oui, je sais, j'ai mis des delay dégueulasse mais c'est dans mon cas, c'est facile et c'est pas grave
      //else
      allumeLed(n, invisible);
      delay(TEMPS_CLIGNETEMENT);
    }
  }



  void arcenciel(uint8_t n)
  {
    for (uint16_t i = 0; i < matrice[n].numPixels(); i++)
      matrice[n].setPixelColor(i, roue(n, (i + cycle[n]) & 255));
    matrice[n].show();
    cycle[n]++;
    if (cycle[n] >= 256)
      cycle[n] = 0;
  }


  uint32_t roue(uint8_t n, byte rouePos)
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

  void setup()
{
  Wire.begin(CONTACT);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  pinMode(CONTACT0_PIN, INPUT);
  pinMode(CONTACT1_PIN, INPUT);
  pinMode(CONTACT2_PIN, INPUT);
  pinMode(CONTACT3_PIN, INPUT);
  pinMode(CONTACT4_PIN, INPUT);
  pinMode(CONTACT5_PIN, INPUT);

  for (uint8_t i = 0; i < MATRICE_NOMBRE; i++)
  {
    matrice[i].begin();
    matrice[i].show();
    matrice[i].setBrightness(LUMINOSITE);
  }

  Serial.begin(9600);
}
