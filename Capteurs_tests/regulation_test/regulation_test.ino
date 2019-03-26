#define Kp 0.002d
#define Ki 0.002d
#define Kd 0.0001d
#define R 0.02d
#define HOLE_NBR 50
#define DURATION 20
#define uS_IN_S 1000

byte oldpulse1 = 0; // détection de flan

double pps; //pulse par secondes
double tps; // tours par secondes
double mesuredSpeed; // vitesse estimée en m/s
double calculatedPWM = 0; // PWM à appliquer au moteur après régulation
double error, derivative, integral, previous_error = 0; // erreur entre la vitesse voulue et la mesurée

unsigned int pot = 0; // valeure du potentiomettre de test
unsigned int desiredSpeed = 0; // vitesse voulue

unsigned long currentTime[2]; // temps actuel permettant de calculer la vitesse
unsigned long millisNow, millisPrev = 0;

unsigned long timerSensor = 0;
unsigned long durationSensor = DURATION;

unsigned long timerPID = 0;
unsigned long durationPID = DURATION;

int nbPulse;

bool flagPulse;

void setup() {
  nbPulse = 0;
  flagPulse = false;
  Serial.begin(2000000); // pour ne pas trop ralentir le micro
  pinMode (2, INPUT);

}

void loop() {

  pot = analogRead(0);
  desiredSpeed =   map(pot, 0, 1023, 0, 3);
  /* Serial.print("desiredSpeed ");
    Serial.println(desiredSpeed); //*/






  // détection de 2 pulses pour faire le calcul de vitesse intantanée (doit encore être modifié car bloquant)
  //int timeout = millis();
  Serial.print("timerSensor ");
  Serial.println(timerSensor);
    Serial.print("durationSensor ");
  Serial.println(durationSensor);
    Serial.print("millis ");
  Serial.println(millis()); //*/
  if (timerSensor + durationSensor < millis()){
    Serial.println("triggered");
    pps = nbPulse*uS_IN_S/DURATION;
    regulationPID();
    timerSensor = millis();
    nbPulse = 0;
  }
  else {
    if (!flagPulse && !digitalRead(2)){
      //Serial.println("added pulse");
      flagPulse = true;
      nbPulse++;
    }
    else if (digitalRead(2)) {
      flagPulse = false;
    }
  }
 
  tps = pps / HOLE_NBR; //calcul du nombre de tours/secondes

  /*Serial.println(tps);
  Serial.println(pps);//*/

  mesuredSpeed = 2 * PI * tps * R; //calcul de la vitesse

  //partie de régulation PID

  
}

void regulationPID() {
  millisNow = millis(); //utile pour le Delta t pour l'integrale et la dérivée
  if (desiredSpeed != 0) {
    error = (desiredSpeed - mesuredSpeed) / desiredSpeed; //proportionnel
    integral = integral + error * (millisNow - millisPrev); //integral
    derivative = (error - previous_error) / (millisNow - millisPrev); //dérivatif
    calculatedPWM = constrain((Kp * error + Ki * integral + Kd * derivative) * 255, 0, 255); // somme du total
    previous_error = error;
  }
  else {
    calculatedPWM = 0;
  }

  analogWrite(5, calculatedPWM); // on passe la commande au moteur


  /* Serial.print("calculatedPWM ");
    Serial.println(calculatedPWM);//*/
   Serial.print("error ");
    Serial.println(error); //*/
    Serial.print("mesuredSpeed ");
    Serial.println(mesuredSpeed); //*/

  millisPrev = millis(); //utile pour le Delta t pour l'integrale et la dérivée
}
