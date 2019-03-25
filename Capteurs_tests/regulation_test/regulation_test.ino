byte oldpulse1 = 0;
double pps[5]; //pulse par secondes
double tps[5]; // tours par secondes
double mesuredSpeed[5]; // vitesse estimée en m/s
double R = 0.02; //Rayon de la roue
unsigned int nbrtrous = 50; // nombre de trou dans la roue
unsigned long currentTime[2];

void setup() {

  Serial.begin(2000000); // pour ne pas trop ralentir le micro
  pinMode (2, INPUT);

}

void loop() {





  for (byte j = 0; j < 5; j++) {
    
    byte yolo = 0;
    
    while (yolo < 2) {
      
      byte pulse1 = digitalRead(2);

      if (pulse1 > oldpulse1) {
     

        currentTime[yolo] = micros();
        
        yolo++;
      }
      oldpulse1 = pulse1;
    }



    pps[j] = 1000000 / (currentTime[1] - currentTime[0]); // le million est pour revenir en pulse/secondes

    tps[j] = pps[j] / nbrtrous; //calcul du nombre de tours/secondes
    
    mesuredSpeed[j] = 2 * PI * tps[j] * R; //calcul de la vitesse

  }
  for (int i = 0; i < 5; i++) {

    Serial.print(mesuredSpeed[i]); //les prints prennent trop de temps ils seront supprimé de la version définitive
    
    Serial.println(" m/s");
  }
}
