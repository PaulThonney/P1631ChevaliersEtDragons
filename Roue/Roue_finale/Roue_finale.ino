// Code de l'Arduino Nano gérant le PWM des moteurs du Minotaure
// Code fortement de la version 2017
// Dany Valado et Domminique Humbert
// 01.05.18 // 01.07.2018
// Ce qu'il reste à ajouter dans le code :
// - l'endométrie
// - une mesure de courant
// - un mode combat, démo et full PWM


#include <Wire.h>
#include "pwmfrequency.h"

#include "math.h"

#define PWM_OUTPUT_MOTOR_L 5 // Pin 5 où sort le pwm du moteur 2
#define INPUT_4_MOTOR_L    3  // Pin 9 pour un sens
#define INPUT_3_MOTOR_L    9  // Pin 3 pour l'autre sens
//Les câbles sont inversés

#define PWM_OUTPUT_MOTOR_R 6 // Pin 6 où sort le pwm du moteur 1
#define INPUT_2_MOTOR_R    10  // Pin 10 pour un sens
#define INPUT_1_MOTOR_R    11  // Pin 11 pour l'autre sens

// Définition des différents mouvements que le robot peut faire
#define TURN_AROUND_LEFT  1 //Tourne sur lui-même sens antihoraire 
#define TURN_LEFT         2 //Tourne à gauche
#define RETREAT_LEFT      3 //Recule à gauche
#define TURN_RIGHT        6 //Tourne à droite
#define RETREAT_RIGHT     7 //Recule à droite
#define TURN_AROUND_RIGHT 8 // Tourne sur lui-même sens horaire

#define PWM_MIN 50
#define PWM_MAX 100
#define PWM_AROUND 100 //tourner sur lui-même

#define ADRESSE_ROUE 19 //adresse I2C
#define ADRESSE_SERVO 20
#define ADRESSE_COULEUR 9

//------------Contrôle de la température-------//demandez à Humbert, moi je sais pas
int valr;
int tempPinr = A2;
int vall;
int tempPina = A3;


//------------Variables reçues par I2C---------//

byte x = 0;     //Position x du joystick
byte y = 0;     //Position y du joystick

bool variableRecu = 0; //pour savoir si des données sont reçue ou pas
bool pauseState = 1; //État "en pause/en jeu"
int adresseMaitre = 0;
int limiteTerrain = 0;

//---------------------------------------------//



long int transf_x = 0; //Position x après conversion
long int transf_y = 0; //Position y après conversion

unsigned int angle; //angle...

unsigned int pwm_left = 0;  //Varie entre 0 et 255 pour fournir un PWM au moteur
unsigned int pwm_right = 0; //Varie entre 0 et 255 pour fournir un PWM au moteur

float pwm_leftAngle = 0;  //Angle du vecteur allant de (0;0) à la position (transf_x;transf_y)
float pwm_rightAngle = 0; // 180°- pwm_leftAngle
//Cela permet de connaître le degré d'inclinaison du joystick

float deltaAngle = 0; // = angle aigu/angle obtus pour connaître un rapport (à voir si on garde ou pas)

int setDirection = 0; // Variable d'état de la direction

void setup()
{
  Serial.begin(9600);   //Serial port for de bugging

  Wire.begin(ADRESSE_ROUE);
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


void convertToMotor(int x, int y)
{
  //Transforme le système de coordonnées du joystick de manière à ce que le point (0;0) soit au centre.
  transf_x = (127 - x);
  transf_y = (y - 127);
}

void checkDirection()
{
  if (adresseMaitre != ADRESSE_SERVO) //si position x et y reçu, conversion en angle
    angle = round((float)atan2(transf_y, transf_x) * 180 / 3.14); // angle de -180° à 180° // float pour calcul à virgule // avec passage de radian à degré
  //Serial.println(angle);

  if (angle >= 170 && angle <= -170)
  {
    setDirection = TURN_AROUND_LEFT;
    //Serial.println("   Turn around left");
  }
  else if (angle < 170 && angle >= 90)
  {
    setDirection = TURN_LEFT;
    // //Serial.println("   Turn left");
  }
  else if (angle > -170 && angle <= -90)
  {
    setDirection = RETREAT_LEFT;
    // //Serial.println("   Retreat left");
  }
  else if (angle < 90 && angle > 10)
  {
    setDirection = TURN_RIGHT;
    //  //Serial.println("   Turn right");
  }
  else if (angle > -90 && angle < -10)
  {
    setDirection = RETREAT_RIGHT;
    // //Serial.println("   Retreat right");
  }
  else if (angle > -10 && angle < 10)
  {
    setDirection = TURN_AROUND_RIGHT;
    //Serial.println("TURN_AROUND_RIGHT");
  }

}

void setPWM() // Fonction appelée après l'attribution de setDirection
{
  //Serial.print(setDirection);
  switch (setDirection)
  {
    case TURN_AROUND_LEFT:

      pwm_left = PWM_AROUND;
      pwm_right = pwm_left; //Même valeur de pwm dans les 2 moteurs pour que le robot tourne sur lui-même
      limitPWM(); // Vérifie si les valeurs de pwm ne dépassent pas le PWM maximum

      digitalWrite(INPUT_1_MOTOR_R,    LOW);
      digitalWrite(INPUT_2_MOTOR_R,    HIGH);
      digitalWrite(INPUT_3_MOTOR_L,    HIGH);
      digitalWrite(INPUT_4_MOTOR_L,    LOW);

      analogWrite(PWM_OUTPUT_MOTOR_L, pwm_left);
      analogWrite(PWM_OUTPUT_MOTOR_R, pwm_right);

      break;

    case TURN_LEFT:

      pwm_right = PWM_MAX;
      pwm_left = pwm_right - round((float)(angle - 90) / 90 * pwm_right); // angle de 0 à 180 passe en pourcetage du PWM droite de la déviation par rapport à l'axe y, capici?
      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    HIGH);
      digitalWrite(INPUT_2_MOTOR_R,    LOW);
      digitalWrite(INPUT_3_MOTOR_L,    HIGH);
      digitalWrite(INPUT_4_MOTOR_L,    LOW);

      analogWrite(PWM_OUTPUT_MOTOR_L, pwm_right);
      analogWrite(PWM_OUTPUT_MOTOR_R, pwm_left);

      break;

    case RETREAT_LEFT:

      pwm_right = PWM_MAX;
      pwm_left = pwm_right + round((float)(angle + 90) / 90 * pwm_right);
      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    LOW);
      digitalWrite(INPUT_2_MOTOR_R,    HIGH);
      digitalWrite(INPUT_3_MOTOR_L,    LOW);
      digitalWrite(INPUT_4_MOTOR_L,    HIGH);

      analogWrite(PWM_OUTPUT_MOTOR_L, pwm_right);
      analogWrite(PWM_OUTPUT_MOTOR_R, pwm_left);

      break;

    case TURN_RIGHT:

      pwm_left = PWM_MAX;
      pwm_right = round((float)angle / 90 * pwm_left); //c'est peut-être plus clair ici, prenez un crayon et un papier au pire et faites les axes x et y avec l'angle
      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    HIGH);
      digitalWrite(INPUT_2_MOTOR_R,    LOW);
      digitalWrite(INPUT_3_MOTOR_L,    HIGH);
      digitalWrite(INPUT_4_MOTOR_L,    LOW);

      analogWrite(PWM_OUTPUT_MOTOR_L, pwm_right);
      analogWrite(PWM_OUTPUT_MOTOR_R, pwm_left);

      break;

    case RETREAT_RIGHT:

      pwm_left = PWM_MAX;
      pwm_right = abs(round((float)angle / 90 * pwm_left));
      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    LOW);
      digitalWrite(INPUT_2_MOTOR_R,    HIGH);
      digitalWrite(INPUT_3_MOTOR_L,    LOW);
      digitalWrite(INPUT_4_MOTOR_L,    HIGH);

      analogWrite(PWM_OUTPUT_MOTOR_L, pwm_right);
      analogWrite(PWM_OUTPUT_MOTOR_R, pwm_left);

      break;

    case TURN_AROUND_RIGHT:

      pwm_left = PWM_AROUND;
      pwm_right = pwm_left; //Même valeur de pwm dans les 2 moteurs pour que le robot tourne sur lui-même
      limitPWM();

      digitalWrite(INPUT_1_MOTOR_R,    HIGH);
      digitalWrite(INPUT_2_MOTOR_R,    LOW);
      digitalWrite(INPUT_3_MOTOR_L,    LOW);
      digitalWrite(INPUT_4_MOTOR_L,    HIGH);

      analogWrite(PWM_OUTPUT_MOTOR_L, pwm_left);
      analogWrite(PWM_OUTPUT_MOTOR_R, pwm_right);

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

void reculer()
{
  if (limiteTerrain == 1)
  {
    pwm_right = PWM_MAX;
    pwm_left = pwm_right;
    limitPWM();
    //il recule
    digitalWrite(INPUT_1_MOTOR_R,    LOW);
    digitalWrite(INPUT_2_MOTOR_R,    HIGH);
    digitalWrite(INPUT_3_MOTOR_L,    LOW);
    digitalWrite(INPUT_4_MOTOR_L,    HIGH);

    analogWrite(PWM_OUTPUT_MOTOR_L, pwm_right);
    analogWrite(PWM_OUTPUT_MOTOR_R, pwm_left);
    delay(500);
    //il débloque les roues
    digitalWrite(INPUT_1_MOTOR_R,    LOW);
    digitalWrite(INPUT_2_MOTOR_R,    LOW);
    digitalWrite(INPUT_3_MOTOR_L,    LOW);
    digitalWrite(INPUT_4_MOTOR_L,    LOW);

    analogWrite(PWM_OUTPUT_MOTOR_R, 0);
    analogWrite(PWM_OUTPUT_MOTOR_L, 0);

    limiteTerrain = 0;
    variableRecu = 0;
  }
}

void loop()
{
  /*
    Serial.print ("gauche:\t");
    Serial.println (pwm_left);
    Serial.print ("droite:\t");
    Serial.println (pwm_right);
  */
  
  valr = analogRead(tempPinr);
  int mvr = ( valr / 1024.0) * 5000;// vraiment, demandez à Humbert j'ai rien compris
  int celr = mvr / 10;

  // Serial.println (celr);
  if (celr > 35) {
    digitalWrite (12, HIGH);
  }
  else {
    digitalWrite (12, LOW);
  }

  vall = analogRead(tempPina);
  int mva = ( vall / 1024.0) * 5000; //c'est peut-être expliqué dans son rapport...
  int cela = mva / 10;
  //  Serial.println (cela);

  if (cela > 35) {
    digitalWrite (2, HIGH);
    digitalWrite (8, HIGH);
    //digitalWrite (12, HIGH); //Allume les ventilateurs
    // digitalWrite (7, LOW);
    // digitalWrite (10, LOW);//Eteint le moteur
  }
  else if (cela < 30) {
    digitalWrite (2, LOW);
    digitalWrite (8, LOW);
    //digitalWrite (12, LOW); //Eteint les ventilateurs
  }
  
    //Serial.println(x);
    //Serial.println(y);
  
  if ((x == 0 && y == 0) || variableRecu == 0) //Si le joystick est "au repos" on laisse libre les moteurs
  {
    digitalWrite(INPUT_1_MOTOR_R,    LOW);
    digitalWrite(INPUT_2_MOTOR_R,    LOW);
    digitalWrite(INPUT_3_MOTOR_L,    LOW);
    digitalWrite(INPUT_4_MOTOR_L,    LOW);

    analogWrite(PWM_OUTPUT_MOTOR_R, 0);
    analogWrite(PWM_OUTPUT_MOTOR_L, 0);
  }
  if (variableRecu) //s'il reçoit un ordre de mouvement on exécute les commandes de moteurs
  {
    //Serial.println("ok");
    convertToMotor(x, y);
    checkDirection();
    setPWM();
    variableRecu = 0;
  }
}

void receiveEvent(int howMany)
{
  Serial.println("i2c entré");
  //Serial.println("howMany: ");
  adresseMaitre = (uint8_t)Wire.read();
  if (adresseMaitre == ADRESSE_SERVO)
  {
    angle = (uint8_t)Wire.read();
    //Serial.println(angle);
  }
  else if (adresseMaitre == ADRESSE_COULEUR)
  {
    limiteTerrain = 1;
  }
  else if (adresseMaitre == ADRESSE_ROUE)
  {
    static bool separatorSeen = false;
    uint8_t posX = 0, posY = 0;
    posX = (uint8_t)Wire.read();
    posY = (uint8_t)Wire.read();
    Serial.println (posX);
    Serial.println (posY);
    /* //utilisé pour tester avec alime limité en ampérage var les pics sont très élevés
      if (posX - x > 20) { //limitation en cas d'accélération
        posX = x + 20;
      }
      if (posY - y < -20) {
        posY = y - 20;
      }
    */
    x = posX;
    y = posY;
  }
  variableRecu = 1;
}
