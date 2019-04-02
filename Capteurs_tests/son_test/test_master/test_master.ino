#include <Wire.h>

int oldbtn1 = 0;
int oldbtn2 = 0;
int oldbtn3 = 0;
int pot = 0;
int soundVal = 0;
int oldsoundVal = 0;
void setup() {
  Wire.begin(); // join i2c bus (address optional for master)
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
}

byte x = 1;
byte y = 2;
byte z = 3;

void loop() {
  int newbtn1 = !digitalRead(2);
  int newbtn2 = !digitalRead(3);
  int newbtn3 = !digitalRead(4);
  pot = analogRead(0);
  soundVal = map(pot, 0, 1023, 60, 15);
if (soundVal != oldsoundVal){
      Wire.beginTransmission(2); // transmit to device #8
    Wire.write(soundVal);              // sends one byte
    Wire.endTransmission();    // stop transmitting
    Serial.println(soundVal);
    oldsoundVal = soundVal;
}
  if (newbtn1 > oldbtn1) {

    Wire.beginTransmission(2); // transmit to device #8
    Wire.write(x);              // sends one byte
    Wire.endTransmission();    // stop transmitting
    Serial.println(x);
  }
  if (newbtn2 > oldbtn2) {
    Wire.beginTransmission(2); // transmit to device #8
    Wire.write(y);              // sends one byte
    Wire.endTransmission();    // stop transmitting
    Serial.println(y);
  }
  if (newbtn3 > oldbtn3) {
    Wire.beginTransmission(2); // transmit to device #8
    Wire.write(z);              // sends one byte
    Wire.endTransmission();    // stop transmitting
    Serial.println(z);
  }
  oldbtn1 = newbtn1;
  oldbtn2 = newbtn2;
  oldbtn3 = newbtn3;
}
