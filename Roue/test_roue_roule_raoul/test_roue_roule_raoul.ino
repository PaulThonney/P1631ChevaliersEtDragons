
// Code de l'Arduino Nano gérant le PWM des moteurs du chevalier
// Nathan Lovo // édité par Domminique Humbert
// 23.05.2017 // 01.05.18
// Ce qu'il reste à ajouter dans le code :
/*
   Le pwm qui part de ~70 à 255 plutôt que de 0 à 255
   Vérifier les fonctions turn right / turn left si elles sont correctes
   Ajouter l'I2C

   Penser à l'attaque "charge" :
    - limiter le pmw pour qu'il n'aille pas au-delà de 180 p.ex.
    - état de la charge à envoyer du master à ce code
    - sitôt que la charge est à 1 => pwm à 255
    - charge à 0 => redéfinition des limites du pwm

*/


#include <Wire.h>
#include "pwmfrequency.h"

#include "math.h"

#define PWM_OUTPUT_MOTOR_L 5 // Pin 4 où sort le pwm du moteur 2
#define INPUT_4_MOTOR_L    9  // Pin 9 pour un sens
#define INPUT_3_MOTOR_L    3  // Pin 3 pour l'autre sens

#define PWM_OUTPUT_MOTOR_R 6 // Pin 7 où sort le pwm du moteur 1
#define INPUT_2_MOTOR_R    10  // Pin 10 pour un sens
#define INPUT_1_MOTOR_R    11  // Pin 11 pour l'autre sens

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

#define ADRESSE_ROUE 19
//------------Contrôle de la température-------//
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


  Serial.begin(9600);
}





void loop()
{
  digitalWrite(INPUT_1_MOTOR_R,    HIGH);
  digitalWrite(INPUT_2_MOTOR_R,    LOW);
  digitalWrite(INPUT_3_MOTOR_L,    HIGH);
  digitalWrite(INPUT_4_MOTOR_L,    LOW);

  analogWrite(PWM_OUTPUT_MOTOR_L, 100);
  analogWrite(PWM_OUTPUT_MOTOR_R, 100);
}

