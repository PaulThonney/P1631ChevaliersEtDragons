
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Wire.h>


//define the pins used
#define CLK 13       // SPI Clock, shared with SD card
#define MISO 12      // Input data, from VS1053/SD card
#define MOSI 11      // Output data, to VS1053/SD card
//Connect CLK, MISO and MOSI to hardware SPI pins.

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin



#define PINTEST 19




Adafruit_VS1053_FilePlayer musicPlayer =
  // create breakout-example object!
  Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
// create shield-example object!
//Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);



int RandomNum = 0;
bool SensorRead = false;

//Variables pour la musique
bool MusicPlaying = false;
unsigned long TimeNowMusic = 0;
int MusicTimeout = 10000;



//Variable for the theme choice  1 = Christmas  2 = Halloween  3 = Easter  4 = Birthday
int theme = 0;
int mega = 0;
int volume = 50;
void setup() {

  Wire.begin(2);                // join i2c bus with address #2
  Wire.onReceive(receiveEvent); // register event
  pinMode(PINTEST, OUTPUT);

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

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(volume, volume);

  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

}

void loop()
{
   // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(volume, volume);
  Serial.println(volume);
  
  if(theme == 0)
      {
             Wire.beginTransmission(1); // transmit to device #8
  Wire.write(1);              // sends one byte
  Wire.endTransmission();    // stop transmitting
      }
if (mega == 1 || mega == 2 || mega == 3 || mega == 4)
    {
      theme = mega ;
    }

if (mega == 11 )
    {
      volume = 60;
    }

if (mega == 12)
    {
      volume =45;
    }

if (mega == 13)
    {
      volume =30;
    }

if (mega == 14)
    {
      volume =15;
    }
    
  if (mega == 5) {
  //  if (SensorRead == false) {
   //   SensorRead = true;
      RandomNum = random(1, 3);
      mega = 60;
      if (MusicPlaying == false)
      {
        TimeNowMusic = millis();
       
        PlayMusic();
        
      }
 //   }
  }
//  else {
//    SensorRead = false;
 // }




  if (MusicPlaying == true)
  {
    if (TimeNowMusic + MusicTimeout <= millis() || mega == 0)
    {
      StopMusic();
    }
  }

} //loop

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {

  mega = Wire.read();    // receive byte as an integer
  Serial.println(mega);         // print the integer
}

void PlayMusic()
{
  if (RandomNum == 1)
  {
    if (theme == 1)
    { Serial.println(F("Playing track 001"));
      musicPlayer.startPlayingFile("/track001.mp3");
      MusicPlaying = true;
    }
    if (theme == 2)
    { Serial.println(F("Playing track 003"));
      musicPlayer.startPlayingFile("/track003.mp3");
      MusicPlaying = true;
    }
    if (theme == 3)
    { Serial.println(F("Playing track 005"));
      musicPlayer.startPlayingFile("/track005.mp3");
      MusicPlaying = true;
    }
    if (theme == 4)
    { Serial.println(F("Playing track 007"));
      musicPlayer.startPlayingFile("/track007.mp3");
      MusicPlaying = true;
    }
  }
  if (RandomNum == 2)
  {
    if (theme == 1)
    { Serial.println(F("Playing track 002"));
      musicPlayer.startPlayingFile("/track002.mp3");
      MusicPlaying = true;
    }
    if (theme == 2)
    { Serial.println(F("Playing track 004"));
      musicPlayer.startPlayingFile("/track004.mp3");
      MusicPlaying = true;
    }
    if (theme == 3)
    { Serial.println(F("Playing track 006"));
      musicPlayer.startPlayingFile("/track006.mp3");
      MusicPlaying = true;
    }
    if (theme == 4)
    { Serial.println(F("Playing track 008"));
      musicPlayer.startPlayingFile("/track008.mp3");
      MusicPlaying = true;
    }
  }

}

void StopMusic()
{
  musicPlayer.stopPlaying();
  MusicPlaying = false;
     Wire.beginTransmission(1); // transmit to device #8
  Wire.write(2);              // sends one byte
  Wire.endTransmission();    // stop transmitting
 
}
