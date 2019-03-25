byte oldpulse1 = 0;
double pps[5]; //pulse par secondes
double tps[5]; // tours par secondes
double avgSpeed[5]; // vitesse estimée en m/s
double R = 0.02;
unsigned long currentTime[2];

void setup() {

  Serial.begin(2000000); // pour ne pas trop ralentir le micro
  pinMode (2, INPUT);

}

void loop() {




  //Serial.println(longueure);
  for (byte j = 0; j < 5; j++) {
    byte yolo = 0;
    while (yolo < 2) {
      byte pulse1 = digitalRead(2);

      //Serial.println(yolo);
      if (pulse1 > oldpulse1) {
        //Serial.println("enter if");

        currentTime[yolo] = micros();
        yolo++;
      }
      oldpulse1 = pulse1;
    }
    //Serial.println("exit while");


    pps[j] = 1000000 / (currentTime[1] - currentTime[0]); // le million est pour revenir en pulse/secondes
    // Serial.println(currentTime[1]-currentTime[0]);
    tps[j] = pps[j] / 50;
    avgSpeed[j] = 2 * PI * tps[j] * R;

  }
  for (int i = 0; i < 5; i++) {
    //Serial.println(pps[i]);
    Serial.print(avgSpeed[i]);
    Serial.println(" m/s");
  }
}
