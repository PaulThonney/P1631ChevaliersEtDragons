#define Kp 0.02d
#define Ki 0.002d
#define Kd 0.01d
#define R 0.02d
#define HOLE_NBR 50
#define ARRAY_SIZE 2
byte oldpulse1 = 0; // détection de flan
double pps; //pulse par secondes
double tps; // tours par secondes
double avgSpeed = 0; // Moyenne de 10 vitesses calculées
double Sumspeed = 0; //Somme temporaire des vitesses pour le calcul de avgSpeed
double mesuredSpeed; // vitesse estimée en m/s
double calculatedPWM = 0; // PWM à appliquer au moteur après régulation
double error, derivative, integral, previous_error = 0; // erreur entre la vitesse voulue et la mesurée
unsigned int pot = 0; // valeure du potentiomettre de test
unsigned int desiredSpeed = 0; // vitesse voulue
unsigned long currentTime[2]; // temps actuel permettant de calculer la vitesse
unsigned long microsNow, microsPrev = 0;

unsigned long timerSensor = 0;
unsigned long durationSensor = 20;

unsigned long timerIntel = 0;
unsigned long durationIntel = 1000;

int nbPulse;
bool flagPulse;

void setup() {

  Serial.begin(2000000); // pour ne pas trop ralentir le micro
  pinMode (2, INPUT);
  analogWrite(5, 200);
}

void loop() {

  pot = analogRead(0);
  desiredSpeed =   map(pot, 0, 1023, 0, 8);
  /* Serial.print("desiredSpeed ");
    Serial.println(desiredSpeed); //*/


  for (byte j = 0; j < ARRAY_SIZE; j++) {

    byte yolo = 0;

    // détection de 2 pulses pour faire le calcul de vitesse intantanée (doit encore être modifié car bloquant)
    //int timeout = millis();
    if (timerSensor + durationSensor > micros()) {
      if (digitalRead(2) == HIGH && flagPulse == false) {
        flagPulse = true;
        nbPulse++;
      }
      if (digitalRead(2) == false && flagPulse == true) {
        flagPulse = false;
      }
    } else {
      if (micros() > timerIntel + durationIntel) {

        timerIntel = millis();
      }

      timerSensor = micros();
      pps = (nbPulse/20)*1000000;

      nbPulse = 0;
    }

    if (micros() > timerPID + durationPID) {
      //analogWrite(MOTORSPEEDPIN, regulationPID(motorObjectiveSpeed, motorMeasuredSpeed));
      regulationPID(motorObjectiveSpeed, motorMeasuredSpeed);
      timerPID = micros();
    }

    
    tps[j] = pps[j] / HOLE_NBR; //calcul du nombre de tours/secondes

    mesuredSpeed[j] = 2 * PI * tps[j] * R; //calcul de la vitesse

  }

  for (int i = 0; i < ARRAY_SIZE; i++) {

    /* Serial.print(mesuredSpeed[i]); //les prints prennent trop de temps ils seront supprimé de la version définitive

      Serial.println(" m/s"); //*/

    Sumspeed += mesuredSpeed[i];
    // Serial.println(Sumspeed);

  }
  avgSpeed = Sumspeed / ARRAY_SIZE;
  Sumspeed = 0;

  //partie de régulation PID
  microsNow = millis(); //utile pour le Delta t pour l'integrale et la dérivée

  if (desiredSpeed != 0) {
    error = (desiredSpeed - avgSpeed) / desiredSpeed; //proportionnel
    integral = integral + error * (microsNow - microsPrev); //integral
    derivative = (error - previous_error) / (microsNow - microsPrev); //dérivatif
    calculatedPWM = constrain((Kp * error + Ki * integral + Kd * derivative) * 255, 0, 255); // somme du total
    previous_error = error;
  }
  else {
    calculatedPWM = 0;
  }

  analogWrite(5, calculatedPWM); // on passe la commande au moteur


  /* Serial.print("calculatedPWM ");
    Serial.println(calculatedPWM);//*/
  /* Serial.print("error ");
    Serial.println(error); //*/
  /* Serial.print("avgSpeed ");
    Serial.println(avgSpeed); //*/

  microsPrev = millis(); //utile pour le Delta t pour l'integrale et la dérivée
}
