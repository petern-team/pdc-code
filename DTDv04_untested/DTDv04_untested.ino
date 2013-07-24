/* Features to be teseted:
- new char_arr configuration and pointer functions
- switched irrecv to 8
- added sendCharArray to PDCsend
- changed PDCreceive to have a transmission_complete variable (seems to work)


/*
This sketch decodes IR messages written with category/ time pairs.
Then it prints an opening sequence to the computer, followed by a 
character number (sometimes) and all the number pairs.
 */

#include <IRremote.h>           // IR remote control library
#include "audioCommPin.h"
#include <PDCreceive.h>
#include <PDCsend.h>
//#include "../../../Tufts/CEEO/summer_13/pdc-code/PDCreceive/PDCreceive.h"

const long PRODUCT_ID = 38301; //"DTD01"
const int redLED = 3;
const int greenLED = 2;

char comp_char_arr[(ARR_SIZE*2)/5];        // adjust size to account for storage strategy
int char_index;

PDCreceive DTDreceive(PRODUCT_ID);
PDCsend DTDsend;
IRrecv irrecv(8);    // create the IR library
decode_results results;

boolean comp_transmission; // turns true when a complete audio-wire transmission has been received

void setup() {
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
  Serial.println("exiting setup");
  
  // initialize sketch variables
  char_index = 0;
}



void loop() {
  
  comp_transmission = checkComplete();
  checkLEDstate();
   
  // check for communication through the audio wire  
  if(comp_transmission) {
    char_index = parseArray(comp_char_arr);
    printArray(comp_char_arr, &char_index);
    
    // maybe should add a way to check if the PDC is syncing before sending info
    DTDsend.sendCharArray(comp_char_arr, char_index);      // TEST THIS
    resetStorage();
    resetChars();
    irrecv.enableIRIn();
  }
  
  DTDreceive.checkIR(irrecv, results);
  
  if(DTDreceive.transmission_complete) {
    if(DTDreceive.PDC_sync) {
      delay(50);
      Serial.println("syncing..");
      for(int i=0;i<1;i++) {                // change this if PDC has trouble receiving the first code
        DTDsend.sendSyncCode(PRODUCT_ID);
        delay(50);
      }
      irrecv.enableIRIn();
      DTDreceive.PDC_sync = false;
    } else {
      DTDreceive.printTransmission();
    }
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

void resetChars() {
  for(int i=0; i<char_index;i++) {
    comp_char_arr[i] = 0;
  }
  char_index = 0;
}  

//counts the number of characters in transmissionArray, plus commas and semicolons

//int countCharacters(int num_cols)
//{
//  int num_chars=0;
//  num_chars=num_cols*4;    // two numbers per column, one comma/semicolon/colon per number
//  return num_chars;a=
//}

/*
 * converts a remote protocol code to a logical key code 
 * (or -1 if no digit received)
 */
