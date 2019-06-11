// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Wire.h>

// define the pins used
//#define CLK 13       // SPI Clock, shared with SD card
#define ADDR_SOUND  22

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

String root = "/SOUNDS";

String hurtSounds[] = {
  "T12.mp3",
  "Villagerhurt1.mp3"
};
String attackSounds[] = {
  "hit.mp3",
  "Villager1.mp3"
};
String dieSounds[] = {
  "Villagerdead.mp3"
};
String otherSounds[] = {
  "0_startup.mp3",
  "1_connected.mp3",
  "2_disconnected.mp3",
  "3_pause.mp3",
  "4_controller_disconnected.mp3",
  "5_controller_connected.mp3"
};
String growlSounds[] = {
  "185594__cylon8472__roar.mp3",
  "332582__cylon8472__beast-roar-lvl4.mp3",
  "366671__cylon8472__monster-growl.mp3"
};

void setup() {
  randomSeed(analogRead(0));
  Wire.begin(ADDR_SOUND);
  Wire.onReceive(receiveEvent);

  Serial.begin(9600);
  Serial.println("Adafruit VS1053 Simple Test");

  if (! musicPlayer.begin()) { // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1);
  }
  Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }

  printDirectory(SD.open("/"), 0);

  musicPlayer.setVolume(20, 20);

  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

}

void loop() {
}

void receiveEvent(int howMany) {
  int data = Wire.read();    // receive byte as an integer
  int trackId = -1;
  if (howMany > 1) {
    trackId = Wire.read();
  }

  if (data == 0) { // son par defaut
    musicPlayer.stopPlaying();
    if (trackId <= -1)
      trackId =  random(0, sizeof(otherSounds) / sizeof(otherSounds[0]));
    String filename = otherSounds[trackId];
    filename = root + "/0_other/" + filename;
    musicPlayer.startPlayingFile(filename.c_str());
  }

  if (data == 1) { // Son quand il est touché
    musicPlayer.stopPlaying();
    if (trackId <= -1)
      trackId =  random(0, sizeof(hurtSounds) / sizeof(hurtSounds[0]));
    String filename = hurtSounds[trackId];
    Serial.println("Playing: " + filename);
    filename = root + "/1_hurt/" + filename;
    musicPlayer.startPlayingFile(filename.c_str());
  }

  if (data == 2) { // son quand il attaque
    musicPlayer.stopPlaying();
    if (trackId <= -1)
      trackId =  random(0, sizeof(attackSounds) / sizeof(attackSounds[0]));
    String filename = attackSounds[trackId];
    filename = root + "/2_attack/" + filename;
    musicPlayer.startPlayingFile(filename.c_str());
  }

  if (data == 3) { // son quand il meurt
    musicPlayer.stopPlaying();
    if (trackId <= -1)
      trackId =  random(0, sizeof(dieSounds) / sizeof(dieSounds[0]));
    String filename = dieSounds[trackId];
    filename = root + "/3_die/" + filename;
    musicPlayer.startPlayingFile(filename.c_str());
  }

  if (data == 4) { // son quand il meurt
    musicPlayer.stopPlaying();
    if (trackId <= -1)
      trackId =  random(0, sizeof(growlSounds) / sizeof(growlSounds[0]));
    String filename = growlSounds[trackId];
    filename = root + "/4_growl/" + filename;
    musicPlayer.startPlayingFile(filename.c_str());
  }

  if (data == 250) {
    musicPlayer.stopPlaying();
  }

  if (data == 255) {
    if (! musicPlayer.paused()) {
      Serial.println("Paused");
      musicPlayer.pausePlaying(true);
    } else {
      Serial.println("Resumed");
      musicPlayer.pausePlaying(false);
    }
  }

}

int listFiles(String dirName, String *buffer, int max) {
  File dir = SD.open(dirName);
  int nb = 0;
  while (true) {
    if (nb >= max && max > -1) {
      break;
    }
    File entry =  dir.openNextFile();
    if (! entry) {
      break;
    }
    if (!entry.isDirectory()) {
      Serial.print(entry.name());
      buffer[nb] = entry.name();
      nb++;
    }
    entry.close();
  }
  return nb;
}

void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      //Serial.println("**nomorefiles**");
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
