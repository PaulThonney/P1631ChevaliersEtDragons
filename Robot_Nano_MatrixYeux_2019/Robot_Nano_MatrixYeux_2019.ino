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
int gotHit = 0;
int speaker = 9;
int animation = 0;
int hitSound = 164;
int recvValue = -1;

int Mi0 = 329;
int Fa0 = 349;
int Fa0b = 370;
int Sol0 = 392;
int La = 440;
int Si = 493;
int Do = 523;
int note = 0;
int son = 0;

void setup() {

  Wire.begin(8); //Begin with adress 8
  Wire.onReceive(receiveEvent); // register event
  //Serial.begin(9600);


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

void winkBoth(int xPupil1, int yPupil1, int xPupil2, int yPupil2)
{
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

void angry()
{
  refresh(matrix0);
  refresh(matrix1);
  drawCurrentPupil(matrix0, 3, 3, LED_YELLOW);
  drawCurrentPupil(matrix1, 3, 3, LED_YELLOW);
  matrix[matrix0].drawBitmap(0, 0, angryLeft_bmp, 8, 8, LED_RED);
  matrix[matrix1].drawBitmap(0, 0, angryRight_bmp, 8, 8, LED_RED);
  matrix[matrix0].writeDisplay();
  matrix[matrix1].writeDisplay();
}

void idleAnim2()
{
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
  else
  {
    refresh(matrix0);
    refresh(matrix1);
    drawCurrentPupil(matrix0, 3, 3, LED_YELLOW);
    drawCurrentPupil(matrix1, 3, 3, LED_YELLOW);
    matrix[matrix0].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix1].drawBitmap(0, 0, wideOpen_bmp, 8, 8, LED_GREEN);
    matrix[matrix0].writeDisplay();
    matrix[matrix1].writeDisplay();
    previousIdle = idle;
    idle = 0;
  }
}
void idleAnim3()
{
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
  }
}

void pauseAnim()
{
  if (gameResumed == 0)
  {
    matrix[matrix1].setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
    matrix[matrix1].setTextSize(1);
    matrix[matrix0].setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
    matrix[matrix0].setTextSize(1);
    matrix[matrix1].setTextColor(LED_RED);
    matrix[matrix0].setTextColor(LED_RED);
    tone(speaker, 880, 40);
    decalage = 7;
    previousCurrentTime = currentTime;
    ancientPosx = posx;
    ancientPosy = posy;
    gameResumed = 1;
    //Serial.print("PAUSE WITH ");
    //Serial.print(currentMillis);
    //Serial.print(" - ");
    //Serial.print(previousMillis);
    //Serial.print(" = ");
    //Serial.println(currentTime);
    //Serial.print("AND INTERVAL = ");
    //Serial.println(interval);
    resetTime();
  }


  if (currentTime % 70 == 0)
  {
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


void shieldBlockAnim()
{
  if (gameResumed == 0 || animation != 1 && animation != 0)
  {

    previousCurrentTime = currentTime;
    ancientPosx = posx;
    ancientPosy = posy;
    gameResumed = 1;
    animation = 1;
    //Serial.print("PAUSE WITH ");
    //Serial.print(currentMillis);
    //Serial.print(" - ");
    //Serial.print(previousMillis);
    //Serial.print(" = ");
    //Serial.println(currentTime);
    //Serial.print("AND INTERVAL = ");
    //Serial.println(interval);
    resetTime();
    hitSound = 164;
  }
  if (currentTime <= 100)
    tone(speaker, hitSound / 2);
  else if (currentTime <= 200)
    noTone(speaker);
  else if ( hitSound <= 293)
  {
    tone(speaker, hitSound);
    hitSound = hitSound + 1;
    //Serial.println(hitSound);
  }
  else
  {
    noTone(speaker);
    animation = 0;
  }

  refresh(matrix0);
  refresh(matrix1);
  matrix[matrix0].drawBitmap(0, 0, shieldBlock_bmp, 8, 8, LED_GREEN);
  matrix[matrix1].drawBitmap(0, 0, shieldBlock_bmp, 8, 8, LED_GREEN);
  matrix[matrix0].writeDisplay();
  matrix[matrix1].writeDisplay();
  posx = 3;
  posy = 3;
  previousIdle = idle;
  idle = 0;

}

void hitAnim(){
  if (gameResumed == 0 || animation != 2 && animation != 0)
  {

    previousCurrentTime = currentTime;
    ancientPosx = posx;
    ancientPosy = posy;
    gameResumed = 1;
    animation = 2;
    //Serial.print("PAUSE WITH ");
    //Serial.print(currentMillis);
    //Serial.print(" - ");
    //Serial.print(previousMillis);
    //Serial.print(" = ");
    //Serial.println(currentTime);
    //Serial.print("AND INTERVAL = ");
    //Serial.println(interval);
    resetTime();
    hitSound = 164;
  }
  if (currentTime <= 100)
    tone(speaker, hitSound / 2);
  else if (currentTime <= 200)
    noTone(speaker);
  else if ( hitSound >= 35)
  {
    tone(speaker, hitSound);
    hitSound = hitSound - 1;
  }
  else
  {
    noTone(speaker);
    animation = 0;
  }

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
  previousIdle = idle;
  idle = 0;

}


void deathAnim()
{

  refresh(matrix0);
  refresh(matrix1);
  matrix[matrix0].drawBitmap(0, 0, dead_bmp, 8, 8, LED_RED);
  matrix[matrix1].drawBitmap(0, 0, dead_bmp, 8, 8, LED_RED);
  matrix[matrix0].writeDisplay();
  matrix[matrix1].writeDisplay();
  posx = 3;
  posy = 3;
  previousIdle = idle;
  idle = 0;

  if (gameResumed == 0 || animation != 3 && animation != 0)
  {

    previousCurrentTime = currentTime;
    ancientPosx = posx;
    ancientPosy = posy;
    gameResumed = 1;
    animation = 3;
    //Serial.print("PAUSE WITH ");
    //Serial.print(currentMillis);
    //Serial.print(" - ");
    //Serial.print(previousMillis);
    //Serial.print(" = ");
    //Serial.println(currentTime);
    //Serial.print("AND INTERVAL = ");
    //Serial.println(interval);
    resetTime();

  }
  jumpNote(Si, Do, 30);
  jumpNote(La, Si, 20);
  jumpNote(Sol0, La, 20);
  jumpNote(Fa0b, Sol0, 40);
  tone(speaker, Mi0);
  delay(1000);
  noTone(speaker);
  animation = 0;
}


void jumpNote(int noteA, int noteB, int moduloNote)
{
  note = noteA;
  tone(speaker, noteA);
  delay(500);
  for (son = 0; son <= 1000; son++)
  {
    tone(speaker, noteA);
    if (son % moduloNote == 0)
    {
      noteA = noteA + 1;
    }
  }
  tone(speaker, noteB);
  delay(475);
  noTone(speaker);
  delay(25);
}


void idleCheck()
{
  switch (idle)
  {
    case 1:
      winkBoth(ancientPosx, ancientPosy, posx, posy);
      break;
    case 2:
      idleAnim2();
      break;
    case 3:
      wink(matrix0, ancientPosx, ancientPosy, ancientPosx, ancientPosy);
      break;
    case 4:
      wink(matrix1, ancientPosx, ancientPosy, ancientPosx, ancientPosy);
      break;
    case 5:
      idleAnim3();
      break;
  }
}
void resumeGame()
{
  /* //Serial.print("GAME RESUMED WITH ");
    //Serial.print(previousMillis);
    //Serial.print(" - ");
    //Serial.print(previousCurrentTime);
    previousMillis = previousMillis - previousCurrentTime;


    //Serial.print(" = ");
    //Serial.print(previousMillis);
    //Serial.print(" = ");
    //Serial.print(currentMillis);
    //Serial.print(" - ");
    //Serial.print(previousMillis);
    //Serial.print(" = ");
    //Serial.println(currentMillis - previousMillis);
    //    if(pause == 1)
    //    {
    //      tone(speaker, 440, 40);
    //    }
    //Serial.print("AND INTERVAL = ");
    //Serial.print(interval);
    //Serial.print(" + ");
    //Serial.print(currentTime);
    //Serial.print(" = ");
    interval = interval + currentTime;

    //Serial.println(interval);
    idle = previousIdle;
  */
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
  idle = 0;
  resetTime();
  gameResumed = 0;
  // idleCheck();
}

void resetTime()
{
  previousMillis = currentMillis;
  //Serial.println("RESET TIME");

}
void timeCheck()
{
  if ((currentMillis - previousMillis) >= interval)
  {

    ancientPosx = posx;
    ancientPosy = posy;
    randomIdle = random(1, 11);
    posx = random(2, 5);
    posy = random(2, 5);
    if (randomIdle <= 4)
    {
      idle = 1;
    }
    else if (randomIdle == 5)
    {
      idle = 2;
    }
    else if (randomIdle <= 7)
    {
      idle = 3;
    }
    else if (randomIdle <= 9)
    {
      idle = 4;
    }
    else
    {
      idle = 5;
    }
    interval = random(1000, 5000);
    resetTime();
  }
}

void loop() {
  currentMillis = millis();
  currentTime = currentMillis - previousMillis;
  hitAnim();
  return;
  //tone(speaker, 200);
  // Si la dernière valeur reçue vaut
  if (recvValue == 1) {
    gotHit = 1;
  } else if (recvValue == 2) {
    gotHit = 2;
  } else if (recvValue == 3) {
    pause = 1;
  } else if (recvValue == 4) {
    // On remet tout à 0 car on restart le jeu
    pause = 0;
    gotHit = 0;
    animation = 0;
  }

  if (pause == 1)
  {
    pauseAnim();
  }
  else if (gotHit == 1 || animation == 1)
  {
    shieldBlockAnim();
  }
  else if (gotHit == 5 || animation == 3)
  {
    deathAnim();
  }
  else if (gotHit == 2 || animation == 2)
  {
    hitAnim();
  }
  else
  {

    if (gameResumed == 1) // Cas où on reprend la partie, on s'est fait touché pendant la pause et le bit d'information de touche
    { // renvoie encore "1" => if(gameResumed == 1 & gotHit == 1){faire en sorte qu'on ne se soit pas pris de dégât}
      resumeGame();      // à voir dans le programme des plaques de contact si le bit "pause" lui est envoyé.
    }
    idleCheck();
    timeCheck();
  }


}

void receiveEvent(int howMany) {
  //Serial.println(" recieve event ");
  //gotHit = Wire.read();    // receive byte as an integer
  recvValue = Wire.read();
  gotHit = recvValue;
  //Serial.println(recvValue);
}

/*
   BUZZER


  // Send 1KHz sound signal...
      // ...for 1sec
*/
