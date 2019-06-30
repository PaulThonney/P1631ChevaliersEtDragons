/* Code de l'Arduino gérant le son des robot
  @author: Paul THONNEY and Maxime SCHARWATH
  DATE: 30.06.19
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

int catSounds[] = {6, 2, 2, 1, 6};

void setup() {
  Wire.begin(ADDR_SOUND);
  Wire.onReceive(receiveEvent);

  Serial.begin(115200);
  Serial.println("Adafruit Sound Board!");

  // softwareserial at 9600 baud
  ss.begin(9600);
  sfx.reset();
  Serial.println("SFX board found");

  list();
}

/*
   @func void list liste les fichiers présents dans la mémoire
   @param null
   @return void
*/
void list() {
  uint8_t files = sfx.listFiles();

  Serial.println("File Listing");
  Serial.println("========================");
  Serial.println();
  Serial.print("Found "); Serial.print(files); Serial.println(" Files");
  for (uint8_t f = 0; f < files; f++) {
    Serial.print(f);
    Serial.print("\tname: "); Serial.print(sfx.fileName(f));
    Serial.print("\tsize: "); Serial.println(sfx.fileSize(f));
  }
  Serial.println("========================");
}

uint8_t piste = 0;
bool play = false;
bool stop = false;

/*
   @func void loopPlay joue le son demandé
   @param null
   @return void
*/
void loopPlay() {
  if (!play)return;
  play = false;
  if (! sfx.playTrack(piste) ) {
    Serial.println("Failed to play track?");
  }
}

/*
   @func void loopStop stop la lecture du son
   @param null
   @return void
*/
void loopStop() {
  if (!stop)return;
  stop = false;
  if (! sfx.stop() ) {
    Serial.println("Failed to stop track?");
  }
}

/*
   @func void loop Est la boucle centrale du code c'est elle qui appelle les fonctions principales
   @param null
   @return void
*/
void loop() {
  loopPlay();
  loopStop();
}

/*
   @func int nbTracks liste le nombre de tracks présent ans la mémoire
   @param int nMax
   @return int num
*/
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

/*
   @func void recieveEvent reçoit les info de l'intelligence centrale et oriente sur la bonne fonction dépendament du type de message
   @param int howMany
   @return void
*/
void receiveEvent(int howMany) {
  if (howMany == 0) {
    //Serial.println("PING");
    return;
  }
  int cat = (int)Wire.read();    // receive byte as an integer
  if (cat >= 250) {
    stop = true;
    return;
  }
  int trackId = -1;
  if (howMany > 1) {
    trackId = (int)Wire.read();
  }
  if (trackId < 0) {
    trackId = random(0, catSounds[cat]);
  }
  piste = nbTracks(cat) + trackId;
  Serial.print("PLAYING:");
  Serial.println(sfx.fileName(piste));
  play = true;
  stop = true;
}
