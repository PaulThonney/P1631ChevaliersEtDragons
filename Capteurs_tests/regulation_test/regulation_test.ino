int oldpulse1 = 0;
int oldpulse2 = 0;
double tour = 0; //faire disparaitre le float, c'est le mal
double longueure_roue = 0.05*PI;
double longueure = 0;
unsigned long pulse = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000);
  pinMode (2, INPUT);
  pinMode (3, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  

  double avgSpeed[5];
//Serial.println(longueure);
  unsigned long currentTime[2];
  for (byte j = 0; j<5; j++){
    byte yolo = 0;
  while (yolo<2) {
  byte pulse1 = digitalRead(2);
  byte pulse2 = digitalRead(3);
  //Serial.println(yolo);  
  if (pulse1 > oldpulse1) {
    //Serial.println("enter if");

    currentTime[yolo] = micros();
    yolo++;
  }
  oldpulse1 = pulse1;
}
//Serial.println("exit while");
  

  avgSpeed[j] = 2000000/(currentTime[1]-currentTime[0]);
  Serial.println(currentTime[1]-currentTime[0]);
  }
  for (int i = 0; i < 5; i++) {
    Serial.println(avgSpeed[i]);
  }
  


}
