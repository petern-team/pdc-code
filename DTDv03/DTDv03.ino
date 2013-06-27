/*
This sketch decodes IR messages written with category/ time pairs.
Then it prints an opening sequence to the computer, followed by a 
character number (sometimes) and all the number pairs.
 */

#include <IRremote.h>           // IR remote control library
#include "audioCommPin.h"
#include "PDCreceive.h"

boolean comp_transmission; // turns true when a complete audio-wire transmission has been received
//boolean PDC_in_transmission; // uses true to indicate that LEDs need to be switched

//const int RECEIVEPIN = 9;     // pin connected to IR detector output
//const int NUMCODES = 11;     //  number of codes in the IR "alphabet"
//const int maxNumberOfCodes = 100;
const String transmission_sequence = "abcdefg";
const int redLED = 3;
const int greenLED = 2;

PDCreceive DTDreceive;
long key;
int val;

IRrecv irrecv(9);    // create the IR library
decode_results results;

//int length;
//int val;
//long transmissionArray[2][maxNumberOfCodes];
//int h_index;
//int v_index;

//IRrecv irrecv(RECEIVEPIN);    // create the IR library
//decode_results results;         // IR data goes here

void setup()
{
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  Serial.begin(9600);
 
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  
//  PDC_in_transmission = false;
  comp_transmission = false;
  
  // initialize variables and timers for audio-wire communication
  resetStorage();
  initAudioPin();  
  val = 1;
  
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
  
  if (irrecv.decode(&results)) {  
    Serial.println("got something");
    DTDreceive.PDC_in_transmission = true;
    Serial.println(results.value, HEX);
    // here if data is received
    irrecv.resume();

//    showReceivedData();
    
    key = DTDreceive.convertCodeToKey(results.value);
    switch (val) {
    case 1: 
      val = DTDreceive.translateCodes1(key);
      break;
    case 2: 
      val = DTDreceive.translateCodes2(key);
      break;  
    case 3:
      val = DTDreceive.translateCodes3(key);
      break;
    }
  }
  
  if(digitalRead(BUTTONPIN)) {
    DTDreceive.resetVariables();
  } 
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
