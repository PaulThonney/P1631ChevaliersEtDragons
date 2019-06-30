/* Code de l'Arduino gérant les plaque de contacts des robot
  Son but est de récolter les informations des contacts et de gèrer les NéoPixels
  @author: Paul THONNEY and Maxime SCHARWATH
  DATE: 30.06.19
*/
#include <Adafruit_NeoPixel.h> //LED RGB
#include <Wire.h> //I2C

#define ADDR_CONTACT 0x11 //adresse de cet arduino


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

#define PIXEL_NUMBER 10 //nombre de LED sur une plaque
#define MATRIX_NUMBER 6 //nombre de plaques de contacts

#define LUMINOSITE 50 //luminosité des LEDs
#define BLINKING_TIME 100


#define MATRIX_RIGHT 0
#define MATRIX_LEFT 2
#define MATRIX_BACK_RIGHT 3
#define MATRIX_BACK_LEFT 1
#define MATRIX_FRONT_RIGHT 5
#define MATRIX_FRONT_LEFT 4

Adafruit_NeoPixel matrice[MATRIX_NUMBER] = {
  Adafruit_NeoPixel(PIXEL_NUMBER, MATRICE0_PIN, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(PIXEL_NUMBER, MATRICE1_PIN, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(PIXEL_NUMBER, MATRICE2_PIN, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(PIXEL_NUMBER, MATRICE3_PIN, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(PIXEL_NUMBER, MATRICE4_PIN, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(PIXEL_NUMBER, MATRICE5_PIN, NEO_GRB + NEO_KHZ800)
};

byte oldContact[MATRIX_NUMBER];

bool contactUsed[MATRIX_NUMBER] = {true, true, true, true, true, true};

uint64_t currentTimes[MATRIX_NUMBER];
uint16_t cycle[MATRIX_NUMBER];

bool getHit = false;
int lastHitZone = 0;
bool isBlinking = false;
unsigned long blinkAt = 0;

uint32_t RED = matrice[0].Color(255, 0, 0);
uint32_t BLUE = matrice[0].Color(0, 0, 255);
uint32_t BLACK = matrice[0].Color(0, 0, 0);

int animMode = 0;
int perviousAnimMode = 0;
int nextAnimMode = -1;
unsigned long animAt = 0;
int durationAnim = 1500;

void setup() {
  pinMode(CONTACT0_PIN, INPUT);
  pinMode(CONTACT1_PIN, INPUT);
  pinMode(CONTACT2_PIN, INPUT);
  pinMode(CONTACT3_PIN, INPUT);
  pinMode(CONTACT4_PIN, INPUT);
  pinMode(CONTACT5_PIN, INPUT);

  for (uint8_t i = 0; i < MATRIX_NUMBER; i++) {
    matrice[i].begin();
    matrice[i].show();
    matrice[i].setBrightness(LUMINOSITE);
  }

  Wire.begin(ADDR_CONTACT);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
}

/*
   @func void requestEvent envoie à l'intelligence centrale le fait qu'il se soit fait toucher et quel contact c'était
   @param null
   @return void
*/
void requestEvent() {
  //Serial.println("REQUEST");
  Wire.write(getHit);
  Wire.write(lastHitZone);
  getHit = false;
}
/*
   @func void recieveEvent reçoit les info de l'intelligence centrale et oriente sur la bonne fonction dépendament du type de message
   @param int howMany
   @return void
*/
void receiveEvent(int howMany) {
  if (howMany == 0) {
    //Serial.println("PING");
    return;
  }
  setMode(Wire.read());
  if (howMany >= 2) {
    nextAnimMode = Wire.read();
    animAt = millis();
  }
  if (howMany >= 3) {
    durationAnim = Wire.read() * 250;
  } else {
    durationAnim = 1500;
  }
}

/*
   @func void setMode set le mode demandé par l'intelligence centrale
   @param null
   @return void
*/
void setMode(int mode) {
  perviousAnimMode = animMode;
  animMode = mode;
  nextAnimMode = -1;
  animAt = 0;
}

/*
   @func void loop Est la boucle centrale du code c'est elle qui appelle les fonctions principales
   @param null
   @return void
*/
void loop() {
  checkContact();
  switch (animMode) {
    default:
    case 0:
      animTracking();
      break;
    case 1:
      animRainbow();
      break;
    case 2:
      animShield();
      break;
    case 3:
      blinkAll(RED);
      break;
    case 4:
      showAll(BLACK);
      break;
  }

  if (nextAnimMode > -1 && millis() > animAt + durationAnim) {
    setMode(nextAnimMode);
  }
}

/*
   @func void checkContact check si un contact est pressé et stocke l'information en vu d'un envoit
   @param null
   @return void
*/
void checkContact() {
  if (animMode == 3) {
    getHit = false;
    return;
  }
  int16_t newContact[MATRIX_NUMBER] = {
    digitalRead(CONTACT0_PIN),
    digitalRead(CONTACT1_PIN),
    digitalRead(CONTACT2_PIN),
    digitalRead(CONTACT3_PIN),
    digitalRead(CONTACT4_PIN),
    digitalRead(CONTACT5_PIN)
  };
  for (uint8_t i = 0; i < MATRIX_NUMBER; i++) {
    //Serial.print(newContact[i]);
    if (newContact[i] == LOW && oldContact[i] == HIGH && contactUsed[i]) {
      if (!getHit) {
        getHit = true;
        lastHitZone = i;
      }
      //Serial.println("contact");
    }
    oldContact[i] = newContact[i];
  }
  //Serial.println();
}

/*
   @func bool animBlink
   @param null
   @return void
*/
bool animBlink(uint32_t color, int duration, int callbackAnim) {
  if (!isBlinking) {
    isBlinking = true;
    blinkAt = millis();
  }
  if (millis() > blinkAt + duration) {
    isBlinking = false;
    animMode = callbackAnim;
    return false;
  }
  blinkAll(color);
  return true;
}

void animTracking() {
  showLed(MATRIX_FRONT_LEFT, BLUE);
  showLed(MATRIX_FRONT_RIGHT, RED);
  showLed(MATRIX_BACK_LEFT, BLUE);
  showLed(MATRIX_BACK_RIGHT, RED);
}

void animRainbow() {
  if ((unsigned long)(millis() - currentTimes[0]) >= 100) {
    currentTimes[0] = millis();
    rainbow(0);
  }
  if ((unsigned long)(millis() - currentTimes[2]) >= 150) {
    currentTimes[2] = millis();
    rainbow(2);
  }
  if ((unsigned long)(millis() - currentTimes[3]) >= 200) {
    currentTimes[3] = millis();
    rainbow(3);
  }
  if ((unsigned long)(millis() - currentTimes[5]) >= 250) {
    currentTimes[5] = millis();
    rainbow(5);
  }
}

void animShield() {
  int v = (sin(millis() / 225.0) + 1) / 2.0 * 155 + 10;
  uint32_t c = matrice[0].Color(0, 0, v);
  for (uint8_t i = 0; i < MATRIX_NUMBER; i++) {
    showLed(i, v);
  }
}


// n est le numero de la matrice, ici 0 à 5
void showLed(uint8_t n, uint32_t color) {
  for (uint8_t i = 0; i < PIXEL_NUMBER; i++) {
    matrice[n].setPixelColor(i, color);
  }
  matrice[n].show();
}

void showAll(uint32_t color) {
  for (uint8_t i = 0; i < MATRIX_NUMBER; i++) {
    showLed(i, color);
  }
}

void blinkLed(uint8_t n, uint32_t color) {
  bool on = (millis() % (BLINKING_TIME * 2) <= BLINKING_TIME);
  showLed(n, (on) ? color : BLACK);
}

void blinkAll(uint32_t color) {
  for (uint8_t i = 0; i < MATRIX_NUMBER; i++) {
    blinkLed(i, color);
  }
}

void rainbow(uint8_t n) {
  for (uint16_t i = 0; i < matrice[n].numPixels(); i++)
    matrice[n].setPixelColor(i, roue(n, (i + cycle[n]) & 255));
  matrice[n].show();
  cycle[n]++;
  if (cycle[n] >= 256)
    cycle[n] = 0;
}


uint32_t roue(uint8_t n, byte rouePos) {
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
