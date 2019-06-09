
// Code de l'Arduino Nano gérant le PWM des moteurs du chevalier
//Ce code pemet de faire des tests
//Il actionne les ventillateurs et une LED
// Dany Valado Domminique Humbert
// 23.06.18


#define PWM_OUTPUT_MOTOR_L 5 // Pin 4 où sort le pwm du moteur 2
#define INPUT_4_MOTOR_L    9  // Pin 9 pour un sens
#define INPUT_3_MOTOR_L    3  // Pin 3 pour l'autre sens

#define PWM_OUTPUT_MOTOR_R 6 // Pin 7 où sort le pwm du moteur 1
#define INPUT_2_MOTOR_R    10  // Pin 10 pour un sens
#define INPUT_1_MOTOR_R    11  // Pin 11 pour l'autre sens

//------------Contrôle de la température-------//
int valr;
int tempPinr = A2;
int vall;
int tempPina = A3;


void setup() 
{
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


void loop()
{
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
  else if (cela < 30) {
    digitalWrite (2, LOW);
    digitalWrite (8, LOW);
    //digitalWrite (12, LOW); //Eteint les ventilateurs
  }

  digitalWrite(INPUT_1_MOTOR_R,    LOW);
  digitalWrite(INPUT_2_MOTOR_R,    LOW);
  digitalWrite(INPUT_3_MOTOR_L,    LOW);
  digitalWrite(INPUT_4_MOTOR_L,    LOW);

  analogWrite(PWM_OUTPUT_MOTOR_R, 0);
  analogWrite(PWM_OUTPUT_MOTOR_L, 0);

}
