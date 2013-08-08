/*
  Loudness sensor board prototype.
  By: Marc Bucchieri
  Started: 7/26/2013
  
  Ultrasound sensor code credited to http://arduino.cc/en/Tutorial/Ping?from=Tutorial.UltrasoundSensor
*/

#include<MemoryFree.h>
#include<EEPROM.h>
#include <PDCsend.h>
#include <PDCreceive.h>
#include <IRremote.h>
#include "button.h"

const int RECEIVEPIN = 9;
const int BUTTON = 2;
const int SENSORPIN = A0;
const int ARR_SIZE = 10;
const int TRANS_ID = 512;
const int STORAGE_START = 250;
int PRODUCT_ID = 27301;     // BRD01, eventually add EEPROM saving and IR writing for this
long DATA_INTERVAL = 50;   // send interval in seconds/100, add a way to save this in EEPROM
int DIVIDE_FACTOR;          // number to divide millis() by for time stamps.
volatile long overflow_10;

long duration, inches, cm;

// initialize to all zeros
unsigned int storage[ARR_SIZE] = {};
unsigned int time_stamps[ARR_SIZE] = {};
byte index;
int eeprom_stor_index;
int eeprom_time_index;
int data_val;

boolean trans_complete;

PDCsend sensorSend;
PDCreceive sensorRecv;
//
IRrecv irrecv(RECEIVEPIN);
decode_results results;

button sync_button(0);

void setup() {
  sensorSend.my_id = PRODUCT_ID;
  Serial.begin(9600);
  
  // set a timer interrupt to happen every 10 milliseconds = 100 hz (fastest sampling rate allowed)
  
    cli();
  //set timer0 interrupt at 1.6kHz
  TCCR1A = 0;// set entire TCCR0A register to 0
  TCCR1B = 0;// same for TCCR0B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1600hz increments
  OCR1A = 2500;// = (16*10^6) / (100*1024) - 1 (must be <256)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set prescaler
  //  TCCR2B |= (1 << CS20);  // no prescaler
  //  TCCR2B |= (1 << CS21);  // 8 prescaler
//  TCCR0B |= (1 << CS01) | (1<<CS00);  // 64 prescaler
  TCCR1B |= (1 << CS11) | (1 << CS10); // timer1 64 prescaler
  //  //  TCCR2B |= (1 << CS21) | (1 << CS20);  // 256 prescaler - not for timer2
//  TCCR0B |= (1 << CS02) | (1 << CS00); // 1024 prescaler - not for timer2
  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
  
  attachInterrupt(sync_button.interrupt_pin, rise_funct, RISING);
  
  index = 0;
  
  // each index is 1 int (2 bytes)
  // index 0 stores the divide_factor, time stamps start at index 1 and stored values start
  // at index 251
  eeprom_time_index = 0;
  eeprom_stor_index = STORAGE_START;
  trans_complete = false;
  overflow_10 = 0;
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
  
  Serial.print("free memory: ");
  Serial.println(freeMemory());
}

void loop() {
  sensorRecv.checkIR(irrecv, results);
  if(sensorRecv.transmission_complete) {
      sensorRecv.printTransmission();
      sensorRecv.resetVariables();
    }
//  if(!trans_complete) {
//    if(sync_button.pressed) {
//      save_times();
//      IRsync();
//      trans_complete = true;
//    } else {
//      if(overflow_10 >= DATA_INTERVAL) {
//        
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
//        overflow_10 = 0;
//      }
//    }
//    
//    // if arduino is running out of room store the collected values in EEPROM
//    // 
//    if(index == ARR_SIZE-1) {
//      save_times();
//    }
//  }
}

// save 
void save_times() {
  for(int i=0;i<index;i++) {
      EEPROM.write(eeprom_time_index*2, highByte(time_stamps[i]));
      EEPROM.write(eeprom_time_index*2+1, lowByte(time_stamps[i]));
      eeprom_time_index++;
      EEPROM.write(eeprom_stor_index*2, highByte(storage[i]));
      EEPROM.write(eeprom_stor_index*2+1, lowByte(storage[i]));
      eeprom_stor_index++;
    }
    index = 0;
}


void IRsync() {
  sensorSend.sendArray();
  long start_send = micros();
  unsigned int incoming_write[2][10] = {};
  unsigned int data_time_temp[2][MAXPAIRS] = {};
  int temp_index = 1;
  int trans_id = TRANS_ID; // gets changed to 0 after the first partial array is sent
                           // so the rest appear as a continuation of the entire array
                           
  Serial.print("irsync, index: "); Serial.println(eeprom_time_index);
  data_time_temp[0][0] = 0;
  data_time_temp[1][0] = DIVIDE_FACTOR;
  for(int i=0;i<eeprom_time_index;i++) {
    if(temp_index == MAXPAIRS) {
      
      start_send = micros();
      sensorSend.createPartialArray(trans_id, data_time_temp, temp_index);
      Serial.print((micros()-start_send)/1000); Serial.println(" millis to create array for first 30");
      start_send = micros();
      sensorSend.sendArray(false);      // false tells the library not to send the EoT character
//      sensorSend.printTransmission();
      trans_id = 0;
      temp_index = 0;
      Serial.print((micros()-start_send)/1000); Serial.println(" millis to send first 30");
      start_send = micros();
    }
    data_time_temp[0][temp_index] = word(EEPROM.read(2*i), EEPROM.read(2*i+1));
//    Serial.print("time: "); Serial.print(data_time_temp[0][temp_index]);
    data_time_temp[1][temp_index] = word(EEPROM.read(2*(i+STORAGE_START)), EEPROM.read(1+2*(i+STORAGE_START)));
//    Serial.print(", data value: "); Serial.println(data_time_temp[1][temp_index]);
    temp_index++;
  }
  Serial.print((micros()-start_send)/1000); Serial.println(" millis to read the rest");
  start_send = micros();
  sensorSend.createPartialArray(trans_id, data_time_temp, temp_index);
//  sensorSend.printTransmission();
  sensorSend.sendArray();
  Serial.print((micros()-start_send)/1000); Serial.println(" millis to send the rest");
  
  // check if the DTD wants a resend of the codes, otherwise clear the stored data
//  for(int i=0; i<50; i++) {
//    irrecv.enableIRIn();
//    sensorRecv.checkIR(irrecv, results);
//    if(sensorRecv.parseTransmission(incoming_write) && sensorRecv.transmission_id == 901)
//      return;
//      // this code will keep sending times until the DTD tells it to stop
////    else
////      IRsync();
//  }
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

ISR(TIMER1_COMPA_vect) {
  overflow_10++;
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
