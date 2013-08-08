/*
  Loudness sensor board prototype.
  By: Marc Bucchieri
  Started: 7/26/2013
  
  Ultrasound sensor code credited to http://arduino.cc/en/Tutorial/Ping?from=Tutorial.UltrasoundSensor
*/

#include <MemoryFree.h>
#include <EEPROM.h>
#include <PDCsend.h>
#include <PDCreceive.h>
#include <IRremote.h>
#include "button.h"

const int RECEIVEPIN = 10;
const int BUTTON = 2;
const int SENSORPIN = A0;
const int ARR_SIZE = 10;
const int LOUD_TRANS_ID = 511;
const int ULTSND_TRANS_ID = 512;
const int STORAGE_START = 250;
int PRODUCT_ID = 27301;     // BRD01, eventually add EEPROM saving and IR writing for this
long DATA_INTERVAL = 1000;   // send interval in milliseconds, add a way to save this in EEPROM
int DIVIDE_FACTOR;          // number to divide millis() by for time stamps.
volatile long overflow_1;

long duration, inches, cm;

// initialize to all zeros
unsigned int storage[ARR_SIZE] = {};
unsigned int time_stamps[ARR_SIZE] = {};
byte index;
int eeprom_stor_index;
int eeprom_time_index;
int data_val;

boolean trans_complete;

//PDCsend sensorSend;
PDCreceive sensorRecv;
//
IRrecv irrecv(RECEIVEPIN);
decode_results results;

button sync_button(0);

void setup() {
//  sensorSend.my_id = PRODUCT_ID;
  Serial.begin(9600);
    Serial.print("free memory: ");
  Serial.println(freeMemory());
  
  // set a timer interrupt to happen every 10 milliseconds = 100 hz (fastest sampling rate allowed)
  
  cli();
  //set timer0 interrupt at 1.6kHz
  TCCR2A = 0;// set entire TCCR0A register to 0
  TCCR2B = 0;// same for TCCR0B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 1600hz increments
  OCR2A = 250;// = (16*10^6) / (64*1024) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set prescaler
  //  TCCR2B |= (1 << CS20);  // no prescaler
  //  TCCR2B |= (1 << CS21);  // 8 prescaler
//  TCCR0B |= (1 << CS01) | (1<<CS00);  // 64 prescaler timer 1
   TCCR2B |= (1 << CS22);    // timer2 64 prescaler
//  TCCR1B |= (1 << CS11) | (1 << CS10); // timer1 64 prescaler
  //  //  TCCR2B |= (1 << CS21) | (1 << CS20);  // 256 prescaler - not for timer2
//  TCCR0B |= (1 << CS02) | (1 << CS00); // 1024 prescaler - not for timer2
  
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei();
  
  attachInterrupt(sync_button.interrupt_pin, rise_funct, RISING);

  // each index is 1 int (2 bytes)
  // index 0 stores the divide_factor, time stamps start at index 1 and stored values start
  // at index 251
  index = 0;
  eeprom_time_index = 0;
  eeprom_stor_index = STORAGE_START;
  trans_complete = false;
  overflow_1 = 0;
  irrecv.enableIRIn();
  
  // if samples are less than one a second report, hundredths of a second; if they're
  // taken b/t once a second and once every ten seconds report tenths of seconds;
  // otherwise only report seconds
  if(DATA_INTERVAL < 1000) 
    DIVIDE_FACTOR = 10;
  else if(DATA_INTERVAL < 10000)
    DIVIDE_FACTOR = 100;
  else 
    DIVIDE_FACTOR = 1000;
  

}

void loop() {
  // check if the docking station is trying to sync
    sensorRecv.checkIR(irrecv, results);
    if(sensorRecv.transmission_complete) {
      if(sensorRecv.PDC_sync) {
        Serial.println("syncing");
        DTDsync();
      }
    }
    
//    // if the button has been pressed, send all collected data
//    if(sync_button.pressed) {
//      save_times();
//      sync_button.pressed = false;
//      trans_complete = true;
//      sendData();
//    }
//    
//    // every DATA_INTERVAL seconds/100 sample the selected sensor
//    if(!trans_complete && overflow_1 >= DATA_INTERVAL) {   
//// BEGIN ULTRASOUND SENSOR CODE
//        pinMode(SENSORPIN, OUTPUT);
//        digitalWrite(SENSORPIN, LOW);
//        delayMicroseconds(2);
//        digitalWrite(SENSORPIN, HIGH);
//        delayMicroseconds(5);
//        digitalWrite(SENSORPIN, LOW);
//      
//        // The same pin is used to read the signal from the PING))): a HIGH
//        // pulse whose duration is the time (in microseconds) from the sending
//        // of the ping to the reception of its echo off of an object.
//        pinMode(SENSORPIN, INPUT);
//        duration = pulseIn(SENSORPIN, HIGH);
//      
//        // convert the time into a distance, eventually adda way to choose the 
//        // measuring system (in. or cm)
////        inches = microsecondsToInches(duration);
//        if(duration > 37000)
//          cm = -1;
//        else
//          cm = microsecondsToCentimeters(duration);
//        
////        Serial.print(inches);
////        Serial.print("in, ");
//        Serial.print(cm);
//        Serial.print("cm");
//        Serial.println();
//        data_val = cm;
//// END ULTRASOUND SENSOR, BEGIN LOUDNESS SENSOR CODE
////        data_val = analogRead(SENSORPIN);
////        Serial.println(data_val);
//// END LOUDNESS SENSOR, BEGIN STORAGE CODE
//        storage[index] = data_val;
//        time_stamps[index] = millis()/ DIVIDE_FACTOR;
//        index++;
//        
//        overflow_1 = 0;
//      }
//
//    // if the arrays are running out of room store the collected values in EEPROM
//    if(index == ARR_SIZE-1) {
//      save_times();
//    }
}

// save the collected data into eeprom
void save_times() {
  for(int i=0;i<index;i++) {
    // stop sampling if eeprom is full
      if(eeprom_time_index == 250) {
        trans_complete = true;
        return;
      }
      EEPROM.write(eeprom_time_index*2, highByte(time_stamps[i]));
      EEPROM.write(eeprom_time_index*2+1, lowByte(time_stamps[i]));
      eeprom_time_index++;
      EEPROM.write(eeprom_stor_index*2, highByte(storage[i]));
      EEPROM.write(eeprom_stor_index*2+1, lowByte(storage[i]));
      eeprom_stor_index++;
    }
    index = 0;
}

// send all of the collected data to the docking station in arrays of size MAXPAIRS
void sendData() {
//  sensorSend.sendArray();
////  long start_send = micros();
////  Serial.print((micros()-start_send)/1000); Serial.println(" millis to create array for first 30");
//  unsigned int data_time_temp[2][MAXPAIRS] = {};
//  int temp_index = 1;
//  int trans_id = ULTSND_TRANS_ID; // gets changed to 0 after the first partial array is sent
//                           // so the rest appear as a continuation of the entire array
//                           
//  Serial.print("irsync, index: "); Serial.println(eeprom_time_index);
//  data_time_temp[0][0] = 0;
//  data_time_temp[1][0] = DIVIDE_FACTOR;
//  for(int i=0;i<eeprom_time_index;i++) {
//    if(temp_index == MAXPAIRS) {
//      sensorSend.createPartialArray(trans_id, data_time_temp, temp_index);
//      sensorSend.sendArray(false);      // false tells the library not to send the EoT character
////      sensorSend.printTransmission();
//      trans_id = 0;
//      temp_index = 0;
//    }
//    data_time_temp[0][temp_index] = word(EEPROM.read(2*i), EEPROM.read(2*i+1));
////    Serial.print("time: "); Serial.print(data_time_temp[0][temp_index]);
//    data_time_temp[1][temp_index] = word(EEPROM.read(2*(i+STORAGE_START)), EEPROM.read(1+2*(i+STORAGE_START)));
////    Serial.print(", data value: "); Serial.println(data_time_temp[1][temp_index]);
//    temp_index++;
//  }
//  sensorSend.createPartialArray(trans_id, data_time_temp, temp_index);
////  sensorSend.printTransmission();
//  sensorSend.sendArray();
//  Serial.println("done sending");
//  waitForConfirm();
}

// DTDsync is called when the docking station initiates a sync with the sensor board
void DTDsync() {
  unsigned int incoming_write[2][10] = {};
  bool quit = false;
//  sensorSend.sendSyncCode();
  irrecv.enableIRIn();
  sensorRecv.resetVariables();
  while(!quit) {
    sensorRecv.checkIR(irrecv, results);
    if(sensorRecv.transmission_complete) {
      // there was an error so ask for a resend
      if(!sensorRecv.parseTransmission(incoming_write)) {
//        sensorSend.sendCommand(801);
      } else {
        interpretCommand(sensorRecv.transmission_id, incoming_write, &quit);
      }
    }
  }
}

// if a legitimate transmission has been received while in sync mode, take some action based
// on its content
void interpretCommand(int incoming_id, unsigned int incoming_write[][10], bool *quit) {
  short command_cat = incoming_id/100;
//  switch(command_cat) {
//    case
}

// wait for the DTD to either request a resend or confirm that codes were received (this
// step should be automated by the web interface)
// if not code is received this function will return to loop without resetting storage
void waitForConfirm() {
  unsigned int incoming_write[2][10] = {};
  // check if the DTD wants a resend of the codes, otherwise clear the stored data
  irrecv.enableIRIn();
  overflow_1 = 0;
  // wait 15 seconds
  while(overflow_1 < 15000) {
    sensorRecv.checkIR(irrecv, results);
    
    // if a confirm code is received (confirm = 7__) then reset variables and return.
    // if a resend code is received (8__) then the function calls itself
    if(sensorRecv.transmission_complete) {
      sensorRecv.printTransmission();
      
      // parseTransmission defines the transmission_id variable, can also be read as a 
      // bool to check for checksum errors
      sensorRecv.parseTransmission(incoming_write);
      if(sensorRecv.transmission_id%700 < 100) {
        Serial.println("received confirm code, about to return");
        sensorRecv.resetVariables();
        resetStorage();
        return;
      } else if(sensorRecv.transmission_id%800 < 100) {
        sensorRecv.resetVariables();
        Serial.println("resending codes");
        sync_button.pressed = true;
        return;
      }
      sensorRecv.resetVariables();
    }
  }
//  Serial.print("overlow_10 = "); Serial.println(overflow_1);
  Serial.println("returning from IRsync with no confirm");
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                      UTILITIES
//////////////////////////////////////////////////////////////////////////////////////////////////

//the interrupt service routine. Calls the rise function in the button1 class and tells
// the teensy to send new values next time through loop
void rise_funct()
{
//  Serial.println("button pressed");
  sync_button.rise();
}

ISR(TIMER2_COMPA_vect) {
  overflow_1++;
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

void resetStorage() {
  eeprom_time_index = 0;
  eeprom_stor_index = STORAGE_START;
//  trans_complete = false;    // uncomment this to continue sampling after sending data
  overflow_1 = 0;
}
