#include <Wire.h>
#include "SoftwareI2C.h"
#include "Adafruit_LEDBackpack_Soft.h"

#define matrix0 0
#define matrix1 1

#define LED_RED 1 //1
#define LED_YELLOW 2 //2
#define LED_GREEN 3 //3
#define winkSound 20
#define MATRIX 4


//Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
Adafruit_BicolorMatrix matrix[2] = {
  Adafruit_BicolorMatrix(A0, 2),
  Adafruit_BicolorMatrix(A0, 2)
};
int otherMatrix = 0;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
int currentTime = 0;
int interval = 5000;
int idle = 0; //au bout de INTERVAL, se met à 1 ou plus si le robot n'a rien reçu.
int randomIdle = 0;
int idleAnim = 0; //vaut aléatoirement entre 0 et [à définir] pour effectuer une des animations idle.
int pause = 0;
int posx = 3;
int posy = 3;
int ancientPosx = 3;
int ancientPosy = 3;
int decalage = 7;
int pauseStarted = 1;
int gameResumed = 0;
int previousIdle = 0;
int previousCurrentTime = 0;
int speaker = 9;
int hitSound = 164;
int recvValue = -1;

int xPosPupil = 0;
int yPosPupil = 0;

byte animation = 0;
bool isPlaying = false;

void setup() {

  Wire.begin(69); //Begin with adress 8
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);


  pinMode(speaker, OUTPUT); // Set buzzer - pin 9 as an output

  matrix[matrix0].begin(0x70);
  matrix[matrix1].begin(0x71);  // pass in the address
}


static const uint8_t PROGMEM
wideOpen_bmp[] =
{ B00111100,
  B01000010,
  B10000001,
  B10000001,
  B10000001,
  B10000001,
  B01000010,
  B00111100,
},

wink1_bmp[] =
{ B00000000,
  B01111110,
  B10000001,
  B10000001,
  B10000001,
  B10000001,
  B01111110,
  B00000000,
},
wink2_bmp[] =
{ B00000000,
  B00000000,
  B01111110,
  B10000001,
  B10000001,
  B01111110,
  B00000000,
  B00000000,
},
wink3_bmp[] =
{ B00000000,
  B00000000,
  B00000000,
  B01111110,
  B10000001,
  B00000000,
  B00000000,
  B00000000,
},
angryLeft_bmp[] =
{ B00110000,
  B01001000,
  B10000100,
  B10000010,
  B10000001,
  B10000001,
  B01100010,
  B00011100,
},
angryRight_bmp[] =
{ B00001100,
  B00010010,
  B00100001,
  B01000001,
  B10000001,
  B10000001,
  B01000110,
  B00111000,
},

shieldBlock_bmp[] =
{ B00000000,
  B00000000,
  B00111100,
  B01000010,
  B10000001,
  B00000000,
  B00000000,
  B00000000,
},
gotHitLeft_bmp[] =
{ B00000000,
  B00000110,
  B00111001,
  B01000001,
  B01000001,
  B01000010,
  B00111100,
  B00000000,
},
gotHitRight_bmp[] =
{ B00000000,
  B01100000,
  B10011100,
  B10000010,
  B10000010,
  B01000010,
  B00111100,
  B00000000,
},
dead_bmp[] =
{ B10000001,
  B01000010,
  B00100100,
  B00011000,
  B00011000,
  B00100100,
  B01000010,
  B10000001,
};

void refresh(int nb_matrix)
{
  matrix[nb_matrix].clear();
  matrix[nb_matrix].setRotation(3);
}

void eyeClosed(int nb_matrix, int color)
{
  matrix[nb_matrix].drawLine(0, 4, 7, 4, color);
}
void drawCurrentPupil(int nb_matrix, int x1, int y1, int color)
{
  matrix[nb_matrix].fillRect(x1, y1, 2, 2, color);
}


void drawCenterPupil(int nb_matrix, int color)
{
  matrix[nb_matrix].fillRect(3, 3, 2, 2, color);
}

void wink(int nb_matrix, int xPupil1, int yPupil1, int xPupil2, int yPupil2)
{
  if (nb_matrix == 0)
    otherMatrix = 1;
  else otherMatrix = 0;

  if (currentTime < 25)
  {
    refresh(nb_matrix);
    refresh(otherMatrix);
    drawCurrentPupil(otherMatrix, xPupil1, yPupil1, LED_YELLOW);
    drawCurrentPupil(nb_matrix, xPupil1, yPupil1, LED_YELLOW);
    matrix[otherMatrix].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[nb_matrix].drawBitmap(0, 0, wink1_bmp, 8, 8, LED_GREEN);
    matrix[otherMatrix].writeDisplay();
    matrix[nb_matrix].writeDisplay();
  }
  else if (currentTime < 50)
  {
    refresh(nb_matrix);
    drawCurrentPupil(nb_matrix, xPupil1, yPupil1, LED_YELLOW);
    matrix[nb_matrix].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[nb_matrix].writeDisplay();
  }
  else if (currentTime < 75)
  {
    refresh(nb_matrix);
    matrix[nb_matrix].drawBitmap(0, 0, wink3_bmp, 8, 8, LED_GREEN);
    matrix[nb_matrix].writeDisplay();
    tone(speaker, winkSound, 50);
  }
  else if (currentTime < 115)
  {
    //xPupil1 = xPupil2;
    //yPupil1 = yPupil2;
    refresh(otherMatrix);
    refresh(nb_matrix);
    drawCurrentPupil(otherMatrix, xPupil2, yPupil2, LED_YELLOW);
    drawCurrentPupil(nb_matrix, xPupil2, yPupil2, LED_YELLOW);
    matrix[otherMatrix].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[nb_matrix].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[otherMatrix].writeDisplay();
    matrix[nb_matrix].writeDisplay();
  }
  else if (currentTime < 165)
  {
    refresh(nb_matrix);
    drawCurrentPupil(nb_matrix, xPupil2, yPupil2, LED_YELLOW);
    matrix[nb_matrix].drawBitmap(0, 0, wink1_bmp, 8, 8, LED_GREEN);
    matrix[nb_matrix].writeDisplay();
  }
  else
  {
    refresh(nb_matrix);
    drawCurrentPupil(nb_matrix, xPupil2, yPupil2, LED_YELLOW);
    matrix[nb_matrix].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[nb_matrix].writeDisplay();
    posx = xPupil2;
    posy = yPupil2;
    previousIdle = idle;
    idle = 0;
  }
  //Serial.print(xPupil1);
  ////Serial.print("    ");
  ////Serial.println(yPupil1);
}

void winkBoth(int xPupil1, int yPupil1, int xPupil2, int yPupil2) {
  if (currentTime < 25)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, xPupil1, yPupil1, LED_YELLOW);
    drawCurrentPupil(matrix1, xPupil1, yPupil1, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink1_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink1_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 50)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, xPupil1, yPupil1, LED_YELLOW);
    drawCurrentPupil(matrix1, xPupil1, yPupil1, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 75)
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wink3_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink3_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 115)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, xPupil2, yPupil2, LED_YELLOW);
    drawCurrentPupil(matrix1, xPupil2, yPupil2, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
    tone(speaker, winkSound, 50);
  }
  else if (currentTime < 165)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, xPupil2, yPupil2, LED_YELLOW);
    drawCurrentPupil(matrix1, xPupil2, yPupil2, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink1_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink1_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, xPupil2, yPupil2, LED_YELLOW);
    drawCurrentPupil(matrix1, xPupil2, yPupil2, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
    previousIdle = idle;
    idle = 0;
  }

}

void angry() {
  refresh(matrix0);
  refresh(matrix1);
  drawCurrentPupil(matrix0, 3, 3, LED_YELLOW);
  drawCurrentPupil(matrix1, 3, 3, LED_YELLOW);
  matrix[matrix0].drawBitmap(0, 0, angryLeft_bmp, 8, 8, LED_RED);
  matrix[matrix1].drawBitmap(0, 0, angryRight_bmp, 8, 8, LED_RED);
  matrix[matrix0].writeDisplay();
  matrix[matrix1].writeDisplay();
}

void idleAnim2() {
  isPlaying = true;
  if (currentTime < 100)
  {
    interval = random(3500, 8000); //l'animation met au total 3450ms. Pour qu'elle s'effectue jusqu'au bout, on redéfinit ici l'interval
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 4, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 4, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink1_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink1_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 1100)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 5, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 5, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 1150)
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wink3_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink3_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
    tone(speaker, winkSound, 50);
  }
  else if (currentTime < 2150)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 5, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 5, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 2200)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 4, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 4, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 2250)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 3, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 3, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 2300)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 2, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 2, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 3300)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 1, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 1, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 3350)
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wink3_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink3_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
    tone(speaker, winkSound, 50);
  }
  else if (currentTime < 3400)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 3, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 3, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink2_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 3450)
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 3, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 3, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wink1_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wink1_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else {
    isPlaying = false;
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 3, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 3, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
}


void idleAnim3() {
  isPlaying = true;
  if (currentTime < 75)
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    drawCurrentPupil(matrix0, 2, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 2, 3, LED_YELLOW);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 575)
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    drawCurrentPupil(matrix0, 1, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 1, 3, LED_YELLOW);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 650)
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    drawCurrentPupil(matrix0, 2, 4, LED_YELLOW);
    drawCurrentPupil(matrix1, 2, 4, LED_YELLOW);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 725)
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    drawCurrentPupil(matrix0, 3, 5, LED_YELLOW);
    drawCurrentPupil(matrix1, 3, 5, LED_YELLOW);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 800)
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    drawCurrentPupil(matrix0, 4, 4, LED_YELLOW);
    drawCurrentPupil(matrix1, 4, 4, LED_YELLOW);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 1300)
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    drawCurrentPupil(matrix0, 5, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 5, 3, LED_YELLOW);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else if (currentTime < 1375)
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    drawCurrentPupil(matrix0, 4, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 4, 3, LED_YELLOW);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
  }
  else
  {
    refresh(matrix0);
    refresh(matrix1);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    drawCurrentPupil(matrix0, 3, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 3, 3, LED_YELLOW);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
    posx = 3;
    posy = 3;
    previousIdle = idle;
    idle = 0;
    isPlaying = false;
  }
}

void pauseAnim() {
  if (gameResumed == 0) {
    matrix[matrix1].setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
    matrix[matrix1].setTextSize(1);
    matrix[matrix0].setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
    matrix[matrix0].setTextSize(1);
    matrix[matrix1].setTextColor(LED_RED);
    matrix[matrix0].setTextColor(LED_RED);
    decalage = 7;
    previousCurrentTime = currentTime;
    ancientPosx = posx;
    ancientPosy = posy;
    gameResumed = 1;
    resetTime();
  }


  if (currentTime % 70 == 0) {
    //resetTime();
    refresh(matrix1);
    matrix[matrix1].setCursor(decalage, 0);
    matrix[matrix1].print("PAUSE");
    matrix[matrix1].writeDisplay();
    refresh(matrix0);
    matrix[matrix0].setCursor(decalage + 15, 0);
    matrix[matrix0].print("PAUSE");
    matrix[matrix0].writeDisplay();
    decalage = decalage - 1;
    if (decalage <= -50)
      decalage = 7;

  }
}


void shieldBlockAnim() {
  closeAnim(1000);
  refresh(matrix0);
  refresh(matrix1);
  matrix[matrix0].drawBitmap(0, 0, shieldBlock_bmp, 8, 8, LED_GREEN);
  matrix[matrix1].drawBitmap(0, 0, shieldBlock_bmp, 8, 8, LED_GREEN);
  matrix[matrix0].writeDisplay();
  matrix[matrix1].writeDisplay();
  posx = 3;
  posy = 3;
}

void hitAnim() {
  closeAnim(1000);
  refresh(matrix0);
  refresh(matrix1);
  matrix[matrix0].drawBitmap(0, 0, gotHitLeft_bmp, 8, 8, LED_RED);
  matrix[matrix1].drawBitmap(0, 0, gotHitRight_bmp, 8, 8, LED_RED);
  drawCurrentPupil(matrix0, 4, 3, LED_YELLOW);
  drawCurrentPupil(matrix1, 2, 3, LED_YELLOW);
  matrix[matrix0].writeDisplay();
  matrix[matrix1].writeDisplay();
  posx = 3;
  posy = 3;
}


void deathAnim() {
  refresh(matrix0);
  refresh(matrix1);
  matrix[matrix0].drawBitmap(0, 0, dead_bmp, 8, 8, LED_RED);
  matrix[matrix1].drawBitmap(0, 0, dead_bmp, 8, 8, LED_RED);
  matrix[matrix0].writeDisplay();
  matrix[matrix1].writeDisplay();
  posx = 3;
  posy = 3;
}

void controlledEyeAnim() {
  refresh(matrix0);
  refresh(matrix1);
  drawCurrentPupil(matrix0, xPosPupil, yPosPupil, LED_YELLOW);
  drawCurrentPupil(matrix1, xPosPupil, yPosPupil, LED_YELLOW);
  matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
  matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
  matrix[matrix0].writeDisplay();
  matrix[matrix1].writeDisplay();
}


void idleCheck() {
  switch (idle) {
    default:
    case 0:
      winkBoth(ancientPosx, ancientPosy, posx, posy);
      break;
    case 1:
      idleAnim2();
      break;
    case 2:
      wink(matrix0, ancientPosx, ancientPosy, ancientPosx, ancientPosy);
      break;
    case 3:
      wink(matrix1, ancientPosx, ancientPosy, ancientPosx, ancientPosy);
      break;
    case 4:
      idleAnim3();
      break;
  }
}

void resumeGame() {
  refresh(matrix0);
  refresh(matrix1);
  matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
  matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
  drawCurrentPupil(matrix0, ancientPosx, ancientPosy, LED_YELLOW);
  drawCurrentPupil(matrix1, ancientPosx, ancientPosy, LED_YELLOW);
  matrix[matrix0].writeDisplay();
  matrix[matrix1].writeDisplay();
  posx = 3;
  posy = 3;
  resetTime();
}

void resetTime() {
  previousMillis = currentMillis;
}

void timeCheck() {
  if ((currentMillis - previousMillis) >= interval) {
    ancientPosx = posx;
    ancientPosy = posy;
    randomIdle = random(1, 11);
    posx = random(2, 5);
    posy = random(2, 5);
    idle = random(0, 4);
    interval = random(1000, 5000);
    resetTime();
  }
}

void defaultAnim() {
  idleCheck();
  timeCheck();
}

unsigned long animStartAt;
void setAnim(byte anim) {
  animStartAt = millis();
  animation = anim;
}

void closeAnim(int duration) {
  if (millis() > animStartAt + duration) {
    setAnim(0);
  }
}

void loop() {
  currentMillis = millis();
  currentTime = currentMillis - previousMillis;
  switch (animation) {
    default:
    case 0: defaultAnim(); break;
    case 1: hitAnim(); break;
    case 2: shieldBlockAnim(); break;
    case 3: deathAnim(); break;
    case 4: pauseAnim(); break;
    case 5: controlledEyeAnim(); break;

  }
}

void setEyePos(int x, int y) {
  xPosPupil = map(x, -3, 3, 0, 6);
  yPosPupil = map(y, -3, 3, 0, 6);
}

void receiveEvent(int howMany) {
  byte anim = Wire.read();
  setAnim(anim);
  if (howMany >= 2) {
    byte data = Wire.read();
    xPosPupil = data & 0b111;
    yPosPupil = (data & 0b111000) >> 3;
  }

  Serial.println(String(anim)+" "+String(xPosPupil)+" "+String(yPosPupil));

}

/*
   BUZZER


  // Send 1KHz sound signal...
      // ...for 1sec
*/
