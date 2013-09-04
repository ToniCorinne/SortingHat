// Based on "Trigger" example sketch for Lilypad MP3 Player
// Mike Grusin, SparkFun Electronics
// http://www.sparkfun.com

// This sketch can output serial debugging information if desired
// by changing the global variable "debugging" to true. Note that
// this will take away trigger inputs 4 and 5, which are shared 
// with the TX and RX lines. You can keep these lines connected to
// trigger switches and use the serial port as long as the triggers
// are normally open (not grounded) and remain ungrounded while the
// serial port is in use.

// Uses the SdFat library by William Greiman, which is supplied
// with this archive, or download from http://code.google.com/p/sdfatlib/

// Uses the SFEMP3Shield library by Bill Porter, which is supplied
// with this archive, or download from http://www.billporter.info/

// License:
// We use the "beerware" license for our firmware. You can do
// ANYTHING you want with this code. If you like it, and we meet
// someday, you can, but are under no obligation to, buy me a
// (root) beer in return.

// Have fun! 
// -your friends at SparkFun

// Revision history:
// 1.0 initial release MDG 2012/11/01
//Modified by Toni Klopfenstein 9/13

// We'll need a few libraries to access all this hardware!

#include <SPI.h>            // To talk to the SD card and MP3 chip
#include <SdFat.h>          // SD card file system
#include <SFEMP3Shield.h>   // MP3 decoder chip

// Constants for the trigger input pins, which we'll place
// in an array for convenience:

int TRIG1 = A0;

// And a few outputs we'll be using:

const int EN_GPIO1 = A2; // Amp enable + MIDI/MP3 mode select
const int SD_CS = 9;     // Chip Select for SD card

// Create library objects:

SFEMP3Shield MP3player;
SdFat sd;

// Set debugging = true if you'd like status messages sent 
// to the serial port. Note that this will take over trigger
// inputs 4 and 5. (You can leave triggers connected to 4 and 5
// and still use the serial port, as long as you're careful to
// NOT ground the triggers while you're using the serial port).

boolean debugging = true;

// Set interrupt = false if you would like a triggered file to
// play all the way to the end. If this is set to true, new
// triggers will stop the playing file and start a new one.

boolean interrupt = false;

// Set interruptself = true if you want the above rule to also
// apply to the same trigger. In other words, if interrupt = true
// and interruptself = false, subsequent triggers on the same
// file will NOT start the file over. However, a different trigger
// WILL stop the original file and start a new one.

boolean interruptself = true;

// We'll store the five filenames as arrays of characters.
// "Short" (8.3) filenames are used, followed by a null character.

char filename[5][13];


void setup()
{
  int x, index;
  SdFile file;
  byte result;
  char tempfilename[13];

//Set trigger as an input for flexiforce resistor
    pinMode(A0,INPUT);

  // The board uses a single I/O pin to select the
  // mode the MP3 chip will start up in (MP3 or MIDI),
  // and to enable/disable the amplifier chip:
  
  pinMode(EN_GPIO1,OUTPUT);
  digitalWrite(EN_GPIO1,LOW);  // MP3 mode / amp off


  // If debugging is true, initialize the serial port:
  // (The 'F' stores constant strings in flash memory to save RAM)
  
  if (debugging)
  {
    Serial.begin(9600);
    Serial.println(F("Lilypad MP3 Player trigger sketch"));
  }
  
  // Initialize the SD card; SS = pin 9, half speed at first

  if (debugging) Serial.print(F("initialize SD card... "));

  result = sd.begin(SD_CS, SPI_HALF_SPEED); // 1 for success
  
  if (result != 1) // Problem initializing the SD card
  {
    if (debugging) Serial.print(F("error, halting"));
    
  }
  else
    if (debugging) Serial.println(F("success!"));
  
  // Start up the MP3 library

  if (debugging) Serial.print(F("initialize MP3 chip... "));

  result = MP3player.begin(); // 0 or 6 for success

  // Check the result, see the library readme for error codes.

  if ((result != 0) && (result != 6)) // Problem starting up
  {
    if (debugging)
    {
      Serial.print(F("error code "));
      Serial.print(result);
      Serial.print(F(", halting."));
    }
  }
  else
    if (debugging) Serial.println(F("success!"));

  // Now we'll access the SD card to look for any (audio) files
  // starting with the characters '1' to '5':

  if (debugging) Serial.println(F("reading root directory"));

  // Start at the first file in root and step through all of them:

  sd.chdir("/",true);
  while (file.openNext(sd.vwd(),O_READ))
  {
    // get filename

    file.getFilename(tempfilename);

    // Does the filename start with char '1' through '5'?      

    if (tempfilename[0] >= '1' && tempfilename[0] <= '5')
    {
      // Yes! subtract char '1' to get an index of 0 through 4.

      index = tempfilename[0] - '1';
      
      // Copy the data to our filename array.

      strcpy(filename[index],tempfilename);
  
      if (debugging) // Print out file number and name
      {
        Serial.print(F("found a file with a leading "));
        Serial.print(index+1);
        Serial.print(F(": "));
        Serial.println(filename[index]);
      }
    }
    else
      if (debugging)
      {
        Serial.print(F("found a file w/o a leading number: "));
        Serial.println(tempfilename);
      }
      
    file.close();
  }

  if (debugging)
    Serial.println(F("done reading root directory"));
  
  if (debugging) // List all the files we saved:
  {
    for(x = 0; x <= 4; x++)
    {
      Serial.print(F("trigger "));
      Serial.print(x+1);
      Serial.print(F(": "));
      Serial.println(filename[x]);
    }
  }

  // Set the VS1053 volume. 0 is loudest, 255 is lowest (off):

  MP3player.setVolume(10,10);
  
  // Turn on the amplifier chip:
  
  digitalWrite(EN_GPIO1,HIGH);
  delay(2);
}


void loop()
{
  int t;   //current trigger
  int x;
  byte result;
  
  // Read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  
  // Print out the value you read:
  Serial.print("Flexiforce analog output: ");
  Serial.println(sensorValue);
  
  //Divide analog range into 4 groups for trigger assignment 
  if (sensorValue >= 0 && sensorValue <255){
    t = 1;
  }
  else if (sensorValue >=256 && sensorValue <511){
    t =  2;
  }
  else if (sensorValue >=512 && sensorValue < 767){
    t = 3;
  }
  else if (sensorValue >=768 && sensorValue <=1023){
    t = 4;
  }
  else {
    t = 5;
  } 
  
   if (debugging)
      {
        Serial.print(F("got trigger "));
        Serial.println(t);
      }
      
      // Do we have a valid filename for this trigger?
      // (Invalid filenames will have 0 as the first character)

      if (filename[t-1][0] == 0)
      {
        if (debugging)
          Serial.println(F("no file with that number"));
      }
      else // We do have a filename for this trigger!
      {
        // Play the filename associated with the trigger number.
        // (If a file is already playing, this command will fail
        //  with error #2).

        result = MP3player.playMP3(filename[t-1]);

        if(debugging)
        {
          if(result != 0)
          {
            Serial.print(F("error "));
            Serial.print(result);
            Serial.print(F(" when trying to play track "));
          }
          else
          {
            Serial.print(F("playing "));
          }
          Serial.println(filename[t-1]);
        }
      }
    }

