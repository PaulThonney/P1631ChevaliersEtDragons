int oldpulse1 = 0;
int oldpulse2 = 0;
double tour = 0; //faire disparaitre le float, c'est le mal
float longueure_roue = 0.05*PI;
unsigned long pulse = 0;
unsigned long actualtime = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode (2, INPUT);
  pinMode (3, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int pulse1 = digitalRead(2);
  int pulse2 = digitalRead(3);
  Serial.println(pulse);

  if (pulse1 > oldpulse1) {

    pulse++;
   tour = pulse/50;
   longueure = tour*longueure_roue;
  
  }
  oldpulse1 = pulse1;
  
  if (millis() > timewhenpressed + 1000) {

  
  
  }


}
