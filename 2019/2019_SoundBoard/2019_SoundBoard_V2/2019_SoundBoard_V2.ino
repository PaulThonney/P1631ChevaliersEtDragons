/*
  Menu driven control of a sound board over UART.
  Commands for playing by # or by name (full 11-char name)
  Hard reset and List files (when not playing audio)
  Vol + and - (only when not playing audio)
  Pause, unpause, quit playing (when playing audio)
  Current play time, and bytes remaining & total bytes (when playing audio)

  Connect UG to ground to have the sound board boot into UART mode


*/
#include <Wire.h>
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"


#define ADDR_SOUND 0x12

int volume;


// Choose any two pins that can be used with SoftwareSerial to RX & TX
#define SFX_TX 6
#define SFX_RX 5

// Connect to the RST pin on the Sound Board
#define SFX_RST 4

// You can also monitor the ACT pin for when audio is playing!

// we'll be using software serial
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);

// pass the software serial to Adafruit_soundboard, the second
// argument is the debug port (not used really) and the third
// arg is the reset pin
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);
// can also try hardware serial with
// Adafruit_Soundboard sfx = Adafruit_Soundboard(&Serial1, NULL, SFX_RST);

int catSounds[] = {2, 6, 4, 2};

void setup() {
  Wire.begin(ADDR_SOUND);
  Wire.onReceive(receiveEvent);

  Serial.begin(115200);
  Serial.println("Adafruit Sound Board!");

  // softwareserial at 9600 baud
  ss.begin(9600);
  // can also do Serial1.begin(9600)

  if (!sfx.reset()) {
    Serial.println("Not found");
    while (1);
  }
  Serial.println("SFX board found");
  pinMode (2, INPUT);
}


void loop() {
}

int nbTracks(int nMax) {
  if (nMax < 0) {
    nMax = sizeof(catSounds) / sizeof(catSounds[0]);
  }
  int num = 0;
  for (int i = 0; i < nMax; i++) {
    num += catSounds[i];
  }
  return num;
}

void receiveEvent(int howMany) {
  if (howMany == 0) {
    //Serial.println("PING");
    return;
  }
  int cat = Wire.read();    // receive byte as an integer
  int trackId = -1;
  if (howMany > 1) {
    trackId = Wire.read();
  }
  if (trackId < 0) {
    trackId = random(0, catSounds[cat]);
  }
  uint8_t piste = nbTracks(cat) + trackId;
  sfx.playTrack(piste);
}
