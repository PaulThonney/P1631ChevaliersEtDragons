// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Wire.h>

// define the pins used
//#define CLK 13       // SPI Clock, shared with SD card
//#define MISO 12      // Input data, from VS1053/SD card
//#define MOSI 11      // Output data, to VS1053/SD card
// Connect CLK, MISO and MOSI to hardware SPI pins.
// See http://arduino.cc/en/Reference/SPI "Connections"

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

Adafruit_VS1053_FilePlayer musicPlayer =
  // create breakout-example object!
  Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
// create shield-example object!
//Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

int data = 0;
int RandomNum = 0;

void setup() {
  randomSeed(analogRead(0));
  //I2C
  Wire.begin(2);                // join i2c bus with address #2
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

  // list files
  printDirectory(SD.open("/"), 0);

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20, 20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

}

void loop() {
  if (data > 14) {
    musicPlayer.setVolume(data, data);
  }
}

void receiveEvent(int howMany) {


  data = Wire.read();    // receive byte as an integer

  if (data == 1) { // Son quand il est touché
    musicPlayer.stopPlaying();
    RandomNum =  random(1, 17);

    if (RandomNum == 1) {
      Serial.println(F("Playing track 001"));
      musicPlayer.startPlayingFile("/track001.mp3");
    }
    if (RandomNum == 2) {
      Serial.println(F("Playing track 002"));
      musicPlayer.startPlayingFile("/track002.mp3");
    }
    if (RandomNum == 3) {
      Serial.println(F("Playing track 003"));
      musicPlayer.startPlayingFile("/track003.mp3");
    }
    if (RandomNum == 4) {
      Serial.println(F("Playing track 004"));
      musicPlayer.startPlayingFile("/track004.mp3");
    }
    if (RandomNum == 5) {
      Serial.println(F("Playing track 005"));
      musicPlayer.startPlayingFile("/track005.mp3");
    }
    if (RandomNum == 6) {
      Serial.println(F("Playing track 006"));
      musicPlayer.startPlayingFile("/track006.mp3");
    }
    if (RandomNum == 7) {
      Serial.println(F("Playing track 007"));
      musicPlayer.startPlayingFile("/track007.mp3");
    }
    if (RandomNum == 8) {
      Serial.println(F("Playing track 008"));
      musicPlayer.startPlayingFile("/track008.mp3");
    }
    if (RandomNum == 9) {
      Serial.println(F("Playing track 009"));
      musicPlayer.startPlayingFile("/track009.mp3");
    }
    if (RandomNum == 10) {
      Serial.println(F("Playing track 010"));
      musicPlayer.startPlayingFile("/track010.mp3");
    }
    if (RandomNum == 11) {
      Serial.println(F("Playing track 011"));
      musicPlayer.startPlayingFile("/track011.mp3");
    }
    if (RandomNum == 12) {
      Serial.println(F("Playing track 012"));
      musicPlayer.startPlayingFile("/track012.mp3");
    }
    if (RandomNum == 13) {
      Serial.println(F("Playing track 013"));
      musicPlayer.startPlayingFile("/track013.mp3");
    }
    if (RandomNum == 14) {
      Serial.println(F("Playing track 014"));
      musicPlayer.startPlayingFile("/track014.mp3");
    }
    if (RandomNum == 15) {
      Serial.println(F("Playing track 015"));
      musicPlayer.startPlayingFile("/track015.mp3");
    }
    if (RandomNum == 16) {
      Serial.println(F("Playing track 016"));
      musicPlayer.startPlayingFile("/track016.mp3");
    }

  }

  if (data == 2) { // son quand il attaque
    
    RandomNum =  random(1, 4);
    
    if (RandomNum == 1) {
      Serial.println(F("Playing track 017"));
      musicPlayer.startPlayingFile("/track017.mp3");
    }
    if (RandomNum == 2) {
      Serial.println(F("Playing track 018"));
      musicPlayer.startPlayingFile("/track018.mp3");
    }
    if (RandomNum == 3) {
      Serial.println(F("Playing track 019"));
      musicPlayer.startPlayingFile("/track019.mp3");
    }
  }

  if (data == 3) {
    if (! musicPlayer.paused()) {
      Serial.println("Paused");
      musicPlayer.pausePlaying(true);
    } else {
      Serial.println("Resumed");
      musicPlayer.pausePlaying(false);
    }
  }

}

/// File listing helper
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
