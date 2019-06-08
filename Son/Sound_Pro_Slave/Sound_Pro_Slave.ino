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

String folders[] = {
  "/other",
  "/hurt",
  "/attack",
  "/die"
};

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);

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

  if (data < sizeof(folders) / sizeof(folders[0])) {
    play(folders[data], trackId);
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

void play(String dir, int id) {
  musicPlayer.stopPlaying();
  if (id <= -1)
    id =  random(0, getDirSize(dir));
  String filename = getFileName(dir, id);
  Serial.println("Playing: " + filename);
  filename = dir + "/" + filename;
  musicPlayer.startPlayingFile(filename.c_str());
}

int getDirSize(String dirName) {
  File dir = SD.open(dirName);
  int nb = 0;
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      break;
    }
    if (!entry.isDirectory()) {
      nb++;
    }
    entry.close();
  }
  return nb;
}

String getFileName(String dirName, int id) {
  File dir = SD.open(dirName);
  int nb = 0;
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      break;
    }
    if (!entry.isDirectory()) {
      if (nb == id) {
        return entry.name();
      }
      nb++;
    }
    entry.close();
  }

}

void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
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
