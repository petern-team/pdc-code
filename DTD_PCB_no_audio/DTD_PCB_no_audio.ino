/*
This sketch decodes IR messages written with category/ time pairs.
Then it prints an opening sequence to the computer, followed by a 
character number (sometimes) and all the number pairs.
 */

#include <IRremote.h>           // IR remote control library
//#include "audioCommPin.h"
#include <RFsend.h>
#include <RFreceive.h>
#include <PDCsend.h>
#include <PDCreceive.h>
#include <VirtualWire.h>
#include <MemoryFree.h>

const unsigned int PRODUCT_ID = 38301; //"DTD01"
const int RECEIVE_PIN = 8;
const int redLED = 3;
const int greenLED = 2;
const int RFRECEIVE = 13;
const int RFTRANSMIT = 12;
const int TRANSMIT_EN_PIN = 4;
const int BUTTONPIN = 7;


//char comp_char_arr[(ARR_SIZE*2)/5];        // adjust size to account for storage strategy
//char last_send[(ARR_SIZE*2)/5];
unsigned int incoming_write[2][10];
//int char_index;
int last_send_len;
int incoming_id;


PDCreceive IR_in;
PDCsend IR_out(PRODUCT_ID);
RFsend RF_out(PRODUCT_ID);
RFreceive RF_in;
IRrecv irrecv(RECEIVE_PIN);    // create the IR library
decode_results results;

boolean comp_transmission; // turns true when a complete audio-wire transmission has been received

void setup() {
  Serial.begin(9600);
  
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
 
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);

  comp_transmission = false;
  
  // RF setup
    vw_set_tx_pin(RFTRANSMIT);
    vw_set_rx_pin(RFRECEIVE);
    vw_set_ptt_pin(TRANSMIT_EN_PIN);
    //vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2400);	 // Bits per sec

    vw_rx_start();       // Start the receiver PLL running
    
   // IR initialize
   irrecv.enableIRIn();
  
  // initialize variables and timers for audio-wire communication
//  resetStorage();
//  initAudioPin();  


  
  // initialize sketch variables
//  char_index = 0;
  incoming_id = 0;
  Serial.print("free memory: ");
  Serial.println(freeMemory());
}



void loop() {
  
//  comp_transmission = checkComplete();
  checkLEDstate();
   
  // check for communication through the audio wire 
  // change this to check if the transmission is meant for IR or RF and hand it 
  // accordingly 
//  if(comp_transmission) {
//    char_index = parseArray(comp_char_arr);
//    Serial.print("char index = "); Serial.println(char_index);
//    printArray(comp_char_arr, &char_index);
    
    // maybe should add a way to check if the PDC is syncing before sending info
    // eventually come up with a cleaner way to change LEDs
//    RF_in.RF_busy = true;
//    checkLEDstate();
//    
//    DTDsend.sendCharArray(comp_char_arr, char_index);      // TEST THIS
//    saveLastSend(comp_char_arr, char_index);
//    resetStorage();
//    resetChars();
//    RF_in.RF_busy = false;
//    irrecv.enableIRIn();
//  }

  IR_in.checkIR(irrecv, results);
  if(IR_in.transmission_complete) {
//    Serial.println("transmission complete");
    if(IR_in.PDC_sync) {
      Serial.println("sending sync code");
      IR_out.sendSyncCode();
      irrecv.enableIRIn();
    } else {
      IR_in.printTransmission();
    }
    IR_in.resetVariables();
  }
  
  // check for RF transmission
  RF_in.checkRF();
  if(RF_in.transmission_complete) {
//    Serial.println("transmission complete");
    if(RF_in.PDC_sync) {
      Serial.println("sending sync code");
      RF_out.sendSyncCode();
    } else {
    // for now there is no sync procedure here
    
    // eventually add a way such as this to automatically resend transmissions
//    if(RF_in.parseTransmission(incoming_write) && RF_in.transmission_id == 800)
//      RF_out.sendCharArray(last_send, last_send_len);
    // automatically send confirmation
//    if(RF_in.parseTransmission(incoming_write)) {
//      incoming_id = RF_in.transmission_id;
//    }
//    if(incoming_id > 500 && incoming_id < 600) {
//      RF_out.sendCommand(incoming_id, true);
//      incoming_id = 0;
//    }
      RF_in.printTransmission();
    }
    RF_in.resetVariables();
  }   
}


void checkLEDstate() {
  if(digitalRead(BUTTONPIN)) {
    IR_in.resetVariables();
    RF_in.resetVariables();
  }
  if((RF_in.RF_busy || IR_in.IR_busy) && digitalRead(greenLED)) {
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
  } else if(!(RF_in.RF_busy || IR_in.IR_busy) && digitalRead(redLED)) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
  }
}

//void resetChars() {
//  for(int i=0; i<char_index;i++) {
//    comp_char_arr[i] = 0;
//  }
//  char_index = 0;
//}  
//
//// save the last character array sent in case a device asks for a resend
//
//void saveLastSend(char char_arr[], int index) {
//  for(int i=0;i<index;i++) {
//    last_send[i]=char_arr[i];
//  }
//  last_send_len = index;
//}

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
