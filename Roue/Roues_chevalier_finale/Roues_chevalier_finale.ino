// Code de l'Arduino Nano gérant le PWM des moteurs du chevalier
// Nathan Lovo // édité par Domminique Humbert et Dany Valado
// 23.05.2017 // 01.05.18 // 01.07.2018

#include <Wire.h>
#include "pwmfrequency.h"

#include "math.h"

#define PWM_OUTPUT_MOTOR_L 6 // Pin où sort le pwm du moteur 2
#define INPUT_4_MOTOR_L   10   // Pin pour un sens
#define INPUT_3_MOTOR_L    11  // Pin pour l'autre sens

#define PWM_OUTPUT_MOTOR_R 5 // Pin où sort le pwm du moteur 1
#define INPUT_2_MOTOR_R    9  // Pin pour un sens
#define INPUT_1_MOTOR_R    3  // Pin pour l'autre sens

// Définition des différents mouvements que le robot peut faire
#define TURN_AROUND_LEFT  1 //Tourne sur lui-même sens antihoraire 
#define TURN_LEFT         2 //Tourne à gauche
#define RETREAT_LEFT      3 //Recule à gauche
#define FORWARD           4 //Avance tout droit
#define BACKWARDS         5 //Recule tout droit
#define TURN_RIGHT        6 //Tourne à droite
#define RETREAT_RIGHT     7 //Recule à droite
#define TURN_AROUND_RIGHT 8 // Tourne sur lui-même sens horaire

#define PWM_MIN 50
#define PWM_MAX 150

//------------Contrôle de la température-------//Humbert, voir programme minotaure finale
int valr;
int tempPinr = A2;
int vall;
int tempPina = A3;


//------------Variables reçues par I2C---------//

int x = 0;     //Position x du joystick
int y = 0;     //Position y du joystick
bool cycle = 0;

int pauseState = 1; //État "en pause/en jeu"

//---------------------------------------------//



long int transf_x = 0; //Position x après conversion
long int transf_y = 0; //Position y après conversion

unsigned int pwm_left = 0;  //Varie entre 0 et 255 pour fournir un PWM au moteur
unsigned int pwm_right = 0; //Varie entre 0 et 255 pour fournir un PWM au moteur

float pwm_leftAngle = 0;  //Angle du vecteur allant de (0;0) à la position (transf_x;transf_y)
float pwm_rightAngle = 0; // 180°- pwm_leftAngle
//Cela permet de connaître le degré d'inclinaison du joystick, afin de

float deltaAngle = 0; // = angle aigu/angle obtus pour connaître un rapport (à voir si on garde ou pas)

int setDirection = 0; // Variable d'état de

void setup() {

  Serial.begin(9600);   //Serial port for de bugging

  Wire.begin(19);
  Wire.onReceive(receiveEvent);

  //Moteur droite
  pinMode(PWM_OUTPUT_MOTOR_R,      OUTPUT);
  pinMode(INPUT_1_MOTOR_R,         OUTPUT);
  pinMode(INPUT_2_MOTOR_R,         OUTPUT);
  //Input 1 et 2 : pont en H du moteur droite

  //Moteur gauche
  pinMode(PWM_OUTPUT_MOTOR_L,      OUTPUT);
  pinMode(INPUT_3_MOTOR_L,         OUTPUT);
  pinMode(INPUT_4_MOTOR_L,         OUTPUT);
  //Input 3 et 4 : pont en H du moteur droite

  digitalWrite(INPUT_1_MOTOR_R,    LOW);
  digitalWrite(INPUT_2_MOTOR_R,    LOW);
  digitalWrite(INPUT_3_MOTOR_L,    LOW);
  digitalWrite(INPUT_4_MOTOR_L,    LOW);
  //Les deux ponts en H sont en HIGH et HIGH => bloqués

  pinMode (8, OUTPUT);
  pinMode (12, OUTPUT);
  pinMode (A2, INPUT);
  pinMode (A3, INPUT);
  digitalWrite(2, LOW); //Eteint la led d'urgence

  //Démarage des ventilateurs

  pinMode (4, INPUT);//désactivation de pin
  pinMode (5, INPUT);//désactivation de pin


  Serial.begin(9600);
}


void convertToMotor(int x, int y) //Transforme les données analogues du joystick en trucs rigolos
{
  //Transforme le système de coordonnées du joystick de manière à ce que le point (0;0) soit au centre.
  transf_x = (512 - x) / 4; //On divise par 4 pour avoir des valeurs plus simples à manipuler
  transf_y = (y - 512) / 4;
  cycle != cycle;
  digitalWrite(2, cycle);
  Serial.println (cycle);
}

void checkDirection() //fonction appelée après conversion des x reçus par le master
{
  //Calcul de l'angle du vecteur (0;0) à la position (transf_x;transf_y)
  pwm_leftAngle = 180 - round(atan2(abs(transf_y), transf_x) * 180 / 3.14);
  pwm_rightAngle = 180 - pwm_leftAngle;

  if (pwm_rightAngle && pwm_leftAngle != 0)
  {
    if (pwm_rightAngle < pwm_leftAngle)
    {
      deltaAngle = (pwm_rightAngle / pwm_leftAngle);
    }
    else
    {
      deltaAngle = (pwm_leftAngle / pwm_rightAngle);
    }
  }
  else deltaAngle = 0;
  /* //Serial.print("deltaAngle = ");
    //Serial.println(deltaAngle);*/

  ////Serial.print de débug/////////////////////
  //  //Serial.print("pwm_leftAngle = ");
  ////Serial.print(pwm_leftAngle);
  //if(transf_y < 0)
  ////Serial.print("   negative");
  //else //Serial.print("   positive");
  ////////////////////////////////////////////


  //Dans les if qui suivent, on observe quel était l'angle d'inclinaison du joystick
  //pour déterminer la direction que doit prendre le robot

  if (pwm_leftAngle <= 20)
  {
    setDirection = TURN_AROUND_LEFT;
    // //Serial.println("   Turn around left");
  }
  else if (pwm_leftAngle > 20 && pwm_leftAngle <= 70)
  {
    if (transf_y >= 5)
    {
      setDirection = TURN_LEFT;
      // //Serial.println("   Turn left");
    }
    else
    {
      setDirection = RETREAT_LEFT;
      // //Serial.println("   Retreat left");
    }
  }
  else if (pwm_leftAngle > 70 && pwm_leftAngle <= 110)
  {
    if (transf_y >= 5)
    {
      setDirection = FORWARD;
      //  //Serial.println("   forward");
    }
    else
    {
      setDirection = BACKWARDS;
      ////Serial.println("   backwards");
    }
  }
  else if (pwm_leftAngle > 110 && pwm_leftAngle <= 160)
  {
    if (transf_y >= 5)
    {
      setDirection = TURN_RIGHT;
      //  //Serial.println("   Turn right");
    }
    else
    {
      setDirection = RETREAT_RIGHT;
      // //Serial.println("   Retreat right");
    }
  }
  else if (pwm_leftAngle > 160)
  {
    setDirection = TURN_AROUND_RIGHT;

  }

}

void setPWM() // Fonction appelée après l'attribution de setDirection
{
  //Serial.print(setDirection);
  switch (setDirection)
  {
    case TURN_AROUND_LEFT:

      //Calcul de la norme du vecteur allant de (0;0) à (transf_x;transf_y) pour déterminer l'inclinaison du joystick

      pwm_left = round(sqrt(transf_x * transf_x + transf_y * transf_y)) * 2; // * 2 pour obtenir des valeurs entre 0 et ~255

      pwm_right = pwm_left; //Même valeur de pwm dans les 2 moteurs pour que le robot tourne sur lui-même

      limitPWM(); // Vérifie si les valeurs de pwm ne dépassent pas 255

      digitalWrite(INPUT_1_MOTOR_R,    LOW);
      digitalWrite(INPUT_2_MOTOR_R,    HIGH);
      digitalWrite(INPUT_3_MOTOR_L,    HIGH);
      digitalWrite(INPUT_4_MOTOR_L,    LOW);

      if (pwm_left > 70)
      {
        analogWrite(PWM_OUTPUT_MOTOR_L, 100);
        analogWrite(PWM_OUTPUT_MOTOR_R, 100);
      }
      break;

    case TURN_LEFT:

      pwm_right = round(sqrt(transf_x * transf_x + transf_y * transf_y)) * 2;
      //pwm_left = abs(round(deltaAngle * pwm_right)) * 2; //deltaAngle * pwm_left ?
      pwm_left = pwm_right / 3;
      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    HIGH);
      digitalWrite(INPUT_2_MOTOR_R,    LOW);
      digitalWrite(INPUT_3_MOTOR_L,    HIGH);
      digitalWrite(INPUT_4_MOTOR_L,    LOW);

      if (pwm_right > 70)
      {
        analogWrite(PWM_OUTPUT_MOTOR_L, pwm_right);
        analogWrite(PWM_OUTPUT_MOTOR_R, pwm_left);
      }
      break;

    case RETREAT_LEFT:

      pwm_right = round(sqrt(transf_x * transf_x + transf_y * transf_y)) * 2;
      //pwm_left = abs(round(deltaAngle * pwm_right)) * 2; //deltaAngle * pwm_left ?
      pwm_left = pwm_right / 3;
      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    LOW);
      digitalWrite(INPUT_2_MOTOR_R,    HIGH);
      digitalWrite(INPUT_3_MOTOR_L,    LOW);
      digitalWrite(INPUT_4_MOTOR_L,    HIGH);

      if (pwm_right > 70)
      {
        analogWrite(PWM_OUTPUT_MOTOR_L, pwm_right);
        analogWrite(PWM_OUTPUT_MOTOR_R, pwm_left);
      }
      break;

    case FORWARD:
      pwm_left = 20 + round(sqrt(transf_x * transf_x + transf_y * transf_y)) * 2;
      pwm_right = pwm_left;

      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    HIGH);
      digitalWrite(INPUT_2_MOTOR_R,    LOW);
      digitalWrite(INPUT_3_MOTOR_L,    HIGH);
      digitalWrite(INPUT_4_MOTOR_L,    LOW);

      if (pwm_left > 70)
      {
        analogWrite(PWM_OUTPUT_MOTOR_L, pwm_left);
        analogWrite(PWM_OUTPUT_MOTOR_R, pwm_right);
      }
      break;

    case BACKWARDS:

      pwm_left = round(sqrt(transf_x * transf_x + transf_y * transf_y)) * 2;
      pwm_right = pwm_left;

      limitPWM();
      digitalWrite(INPUT_1_MOTOR_R,    LOW);
      digitalWrite(INPUT_2_MOTOR_R,    HIGH);
      digitalWrite(INPUT_3_MOTOR_L,    LOW);
      digitalWrite(INPUT_4_MOTOR_L,    HIGH);

      if (pwm_left > 70)
      {
        analogWrite(PWM_OUTPUT_MOTOR_L, pwm_left);
        analogWrite(PWM_OUTPUT_MOTOR_R, pwm_right);
      }
      break;

    case TURN_RIGHT:

      pwm_left = round(sqrt(transf_x * transf_x + transf_y * transf_y)) * 2;
      //pwm_right = abs(round(deltaAngle * pwm_left)) * 2; //pwm du moteur droite doit être plus bas que le pwm du moteur gauche
      pwm_right = pwm_left / 3;
      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    HIGH);
      digitalWrite(INPUT_2_MOTOR_R,    LOW);
      digitalWrite(INPUT_3_MOTOR_L,    HIGH);
      digitalWrite(INPUT_4_MOTOR_L,    LOW);
      if (pwm_left > 70)
      {
        analogWrite(PWM_OUTPUT_MOTOR_L, pwm_right);
        analogWrite(PWM_OUTPUT_MOTOR_R, pwm_left);
      }
      break;

    case RETREAT_RIGHT:

      pwm_left = round(sqrt(transf_x * transf_x + transf_y * transf_y)) * 2;
      //pwm_right = abs(round(deltaAngle * pwm_left)) * 2;
      pwm_right = pwm_left / 3;
      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    LOW);
      digitalWrite(INPUT_2_MOTOR_R,    HIGH);
      digitalWrite(INPUT_3_MOTOR_L,    LOW);
      digitalWrite(INPUT_4_MOTOR_L,    HIGH);
      if (pwm_left > 70)
      {
        analogWrite(PWM_OUTPUT_MOTOR_L, pwm_right);
        analogWrite(PWM_OUTPUT_MOTOR_R, pwm_left);
      }
      break;

    case TURN_AROUND_RIGHT:

      pwm_left = round(sqrt(transf_x * transf_x + transf_y * transf_y)) * 2;
      pwm_right = pwm_left;

      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    HIGH);
      digitalWrite(INPUT_2_MOTOR_R,    LOW);
      digitalWrite(INPUT_3_MOTOR_L,    LOW);
      digitalWrite(INPUT_4_MOTOR_L,    HIGH);

      if (pwm_left > 70)
      {
        analogWrite(PWM_OUTPUT_MOTOR_L, 100);
        analogWrite(PWM_OUTPUT_MOTOR_R, 100);
      }
      break;
  }


}
void limitPWM()
{
  if (pwm_left > PWM_MAX)
  {
    ////Serial.print("PWM TOO HIGH : ");
    ////Serial.println(pwm_left);
    pwm_left = PWM_MAX;
  }
  if (pwm_right > PWM_MAX)
  {
    //Serial.print("PWM TOO HIGH : ");
    //Serial.println(pwm_right);
    pwm_right = PWM_MAX;
  }
}



void loop() {
 


Wire.beginTransmission (14);
Wire.write(1);
Wire.endTransmission();
Serial.println("1");
delay(100);
Wire.beginTransmission (14);
Wire.write(0);
Wire.endTransmission();
Serial.println("0");



  
  
  // Serial.print ("gauche:   ");
  //Serial.println (pwm_left);
  // Serial.print ("droite:    ");
  //Serial.println (pwm_right);

  valr = analogRead(tempPinr);
  int mvr = ( valr / 1024.0) * 5000;
  int celr = mvr / 10;

 // Serial.println (celr);
  if (celr > 35) {
    digitalWrite (12, HIGH);
  }
  else {
    digitalWrite (12, LOW);
  }

  vall = analogRead(tempPina);
  int mva = ( vall / 1024.0) * 5000;
  int cela = mva / 10;
 //  Serial.println (cela);

  if (cela > 35) {
    digitalWrite (2, HIGH);
    digitalWrite (8, HIGH);
    //digitalWrite (12, HIGH); //Allume les ventilateurs
    // digitalWrite (7, LOW);
    // digitalWrite (10, LOW);//Eteint le moteur
  }
  else if (cela <30){
    digitalWrite (2, LOW);
    digitalWrite (8, LOW);
    //digitalWrite (12, LOW); //Eteint les ventilateurs
  }

  if (x == 0 && y == 0)
    return;

  convertToMotor(x, y);
  checkDirection();

  if (transf_y >= 5 || transf_y <= -5 || transf_x >= 5 || transf_x <= -5) //Seuil du joystick à dépasser pour activer les PWM
  {
    //A AJOUTER AVEC I2C//////////////////
    //if(pauseState == 0)              //
    //{                                 //
    setPWM();//GARDER CETTE FONCTION//
    //}                                 //
    //////////////////////////////////////
  }
  else //Si le joystick est "au repos", on bloque les moteurs, on laisse libre les moteurs
  {
    digitalWrite(INPUT_1_MOTOR_R,    LOW);
    digitalWrite(INPUT_2_MOTOR_R,    LOW);
    digitalWrite(INPUT_3_MOTOR_L,    LOW);
    digitalWrite(INPUT_4_MOTOR_L,    LOW);

    analogWrite(PWM_OUTPUT_MOTOR_R, 0);
    analogWrite(PWM_OUTPUT_MOTOR_L, 0);
  }


}


////////////////////PARTIE I2C////////////////////
void receiveEvent(int howMany)
{
  /*//Serial.println("howMany: ");
    //Serial.print(howMany);*/
  static bool separatorSeen = false;
  uint32_t posX = 0, posY = 0;

  for (int i = 0; i < howMany; i++) {
    if (i < 2) {
      posX <<= 8;
      posX |= (uint8_t)Wire.read();
    } else {
      posY <<= 8;
      posY |= (uint8_t)Wire.read();
    }  Serial.println (posX);
  }

  /*
    if (posX - x > 20) { //limitation en cas d'accélération
      posX = x + 20;
    }
    if (posY - y < -20) {
      posY = y - 20;
    }
  */
  x = 1024 - posX;
  y = posY;

}
/////////////////////////////////////////////////////////////



