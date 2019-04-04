#include <Wire.h>

//Memoire pour la communication avec l'esclave
int themeSent; //Le dernier theme envoyee a l'esclave
int volSent;   //Le dernier volume envoyee a l'esclave
bool musicSent = false; //La musique a été envoyée à l'esclave
bool offSent = false; //L'instruction d'arreter la musique a été envoyée
int slaveMessage = 0; //Message reçu par l'arduino esclave
int theme = 3;
int soundVal = 0;
int pot = 0;
bool soundMute = 0;
int oldbtn1 = 0;
int oldbtn2 = 0;
int oldbtn3 = 0;

void setup() {
  //Setup pour la communication maitre-esclave
  Wire.begin(1);
  Wire.onReceive(receiveEvent);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  Serial.begin(9600);

}
void loop() {
  pot = analogRead(0);
  soundVal = map(pot, 0, 1023, 60, 15);

  int newbtn1 = !digitalRead(2);
  int newbtn2 = !digitalRead(3);
  int newbtn3 = !digitalRead(4);

  //Si le theme a etee changee, alors envoyer le nouveau theme a l'esclave
  if ((theme != themeSent) || (slaveMessage == 1) || (soundVal != volSent)) {
    Wire.beginTransmission(2);
    Wire.write(theme);
    Wire.endTransmission();
    themeSent = theme;
    Serial.println("Theme sent");
  }
  if (newbtn1 > oldbtn1) {
    soundMute = true;
  }

  if ( (soundVal != volSent) || ( (slaveMessage == 1) && (theme = themeSent) ) ) {
    Wire.beginTransmission(2);
    Wire.write(10 + soundVal);
    Wire.endTransmission();
    volSent = soundVal;
    slaveMessage = 0;
    Serial.println("Volume sent");
  }

  //Dit a l'esclave de jouer la musique
  if ((newbtn2 > oldbtn2) ) {
    Wire.beginTransmission(2);
    Wire.write(5);
    Wire.endTransmission();

    Serial.println("PLAY!");
  }

  //Dit a l'esclave d'arreter de jouer de la musique
  if ((newbtn3> oldbtn3)) {
    Wire.beginTransmission(2);
    Wire.write(0);
    Wire.endTransmission();
 
    Serial.println("STOP!");
  }
  //Remet a zero le fait que le message a ete envoyee
  else {
    offSent = false;
  }


  //L'esclave envoie un 2 quand il a finit de jouer la musique
  if (slaveMessage == 2) {
    musicSent = false;  //Permet de jouer une nouvelle chanson
    slaveMessage = 0;
  }
  oldbtn1 = newbtn1;
  oldbtn2 = newbtn2;
  oldbtn3 = newbtn3;
}
void receiveEvent(int howMany) {
  slaveMessage = Wire.read();
  Serial.println(slaveMessage);
}
