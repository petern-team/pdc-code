/*
  Loudness sensor board prototype.
  By: Marc Bucchieri
  Started: 7/26/2013
*/

#include<MemoryFree.h>
#include <PDCsend.h>
#include <PDCreceive.h>
#include <IRremote.h>
#include "button.h"

const int RECEIVEPIN = 9;
const int BUTTON = 2;
const int SENSORPIN = A0;
const int ARR_SIZE = 50;
const int TRANS_ID = 511;
int PRODUCT_ID = 27301;     // BRD01, eventually add EEPROM saving and IR writing for this
long DATA_INTERVAL = 2000;   // send intervall in millis, add a way to save this in EEPROM
int DIVIDE_FACTOR;          // number to divide millis() by for time stamps.
unsigned int storage[ARR_SIZE];
unsigned int time_stamps[ARR_SIZE];
byte index;

boolean trans_complete;

PDCsend sensorSend;
//PDCreceive sensorRecv;
//
//IRrecv irrecv(RECEIVEPIN);
//decode_results results;

button sync_button(0);

void setup() {
  sensorSend.my_id = PRODUCT_ID;
  Serial.begin(9600);
  attachInterrupt(sync_button.interrupt_pin, rise_funct, RISING);
  
  for(int i=0;i<ARR_SIZE;i++) {
    storage[i] = 0;
    time_stamps[i] = 0;
  }
  index = 1;
  trans_complete = false;
  
  // if samples are less than one a second report, hundredths of a second; if they're
  // taken b/t once a second and once every ten seconds report tenths of seconds;
  // otherwise only report seconds
  if(DATA_INTERVAL < 1000) 
    DIVIDE_FACTOR = 10;
  else if(DATA_INTERVAL < 10000)
    DIVIDE_FACTOR = 100;
  else 
    DIVIDE_FACTOR = 1000;
  
  storage[0] = DIVIDE_FACTOR; // the first pair sent will be time 0 and the divide_factor
  
  Serial.print("free memory: ");
  Serial.println(freeMemory());
}

void loop() {
  if(!trans_complete) {
    if(sync_button.pressed) {
  //    Serial.println("button pressed");
      IRsync();
      trans_complete = true;
    } else {
      Serial.println(analogRead(SENSORPIN));
      storage[index] = analogRead(SENSORPIN);
      time_stamps[index] = millis()/ DIVIDE_FACTOR;
      index++;
      delay(DATA_INTERVAL);
    }
  }
}

void IRsync() {
  Serial.print("irsync, index: "); Serial.println(index);
  sensorSend.createDataArray(TRANS_ID, storage, time_stamps, index);
  sensorSend.printTransmission();
  sensorSend.sendArray();

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                      UTILITIES
//////////////////////////////////////////////////////////////////////////////////////////////////

//the interrupt service routine. Calls the rise function in the button1 class and tells
// the teensy to send new values next time through loop
void rise_funct()
{
//  Serial.println("button pressed");
  noInterrupts();
  sync_button.rise();
  interrupts();
}
