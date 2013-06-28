/*
This sketch decodes IR messages written with category/ time pairs.
Then it prints an opening sequence to the computer, followed by a 
character number (sometimes) and all the number pairs.
 */

#include <IRremote.h>           // IR remote control library
#include "audioCommPin.h"
#include "PDCreceive.h"

const String transmission_sequence = "abcdefg";
const int redLED = 3;
const int greenLED = 2;

PDCreceive DTDreceive;
IRrecv irrecv(9);    // create the IR library
decode_results results;

boolean comp_transmission; // turns true when a complete audio-wire transmission has been received

void setup()
{
  Serial.begin(9600);
  
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
 
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);

  comp_transmission = false;
  
  // initialize variables and timers for audio-wire communication
  resetStorage();
  initAudioPin();  
  irrecv.enableIRIn();
  
  // initialize sketch variables
}



void loop()
{
  
  comp_transmission = checkTransmission();
  checkLEDstate();
   
  // check for communication through the audio wire  
  if(comp_transmission) {
    parseArray();
    printArray();
    resetStorage();
    resetChars();
  }
  
  DTDreceive.checkIR(irrecv, results);
  
//  if(digitalRead(BUTTONPIN))
//    DTDreceive.resetVariables();
}

void checkLEDstate() {
  if(DTDreceive.PDC_in_transmission && digitalRead(greenLED)) {
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
  } else if(!DTDreceive.PDC_in_transmission && digitalRead(redLED)) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
  }
}

//counts the number of characters in transmissionArray, plus commas and semicolons

//int countCharacters(int num_cols)
//{
//  int num_chars=0;
//  num_chars=num_cols*4;    // two numbers per column, one comma/semicolon/colon per number
//  return num_chars;
//}

/*
 * converts a remote protocol code to a logical key code 
 * (or -1 if no digit received)
 */
