/*
  Loudness sensor board prototype.
  By: Marc Bucchieri
  Started: 7/26/2013
  
  Ultrasound sensor code credited to http://arduino.cc/en/Tutorial/Ping?from=Tutorial.UltrasoundSensor
*/

#include <MemoryFree.h>
#include <EEPROM.h>
#include <VirtualWire.h>
#include <RFsend.h>
#include <RFreceive.h>
#include "button.h"

const int RECEIVEPIN = 9;
const int RFTRANSMIT = 12;
const int RFRECEIVE = 11;
const int TRANSMIT_EN_PIN = 13;    // doesn't really do anything
const int BUTTON = 2;
const int SENSORPIN = A0;
const int ARR_SIZE = 10;
const int LOUD_TRANS_ID = 511;
const int ULTSND_TRANS_ID = 512;
const int STORAGE_START = 200;
const int ID_ADRS = 900;
const int SENS_ADRS = 903;
const int DATA_INT_ADRS = 906;

byte SENSOR_TYPE;        // initializes as ultrasound sensor
int PRODUCT_ID;     // BRD01, eventually add EEPROM saving and IR writing for this
unsigned int DATA_INTERVAL;   // send interval in milliseconds, add a way to save this in EEPROM
                              // sampling rate must be between 100hz and 1 per minute
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

volatile boolean sampling;

RFsend sensorSend;
RFreceive sensorRecv;
////
//IRrecv irrecv(RECEIVEPIN);
//decode_results results;

button sync_button(0);

void setup() {
  Serial.begin(9600);
  
  // load the stored product ID, sensor type, and sampling rate
  loadOldInfo(ID_ADRS);
  loadOldInfo(SENS_ADRS);
  loadOldInfo(DATA_INT_ADRS);
  
  // set a timer interrupt to happen every millisecond (fastest sampling rate allowed for sensor)
  
  cli();
  //set timer0 interrupt at 1.6kHz
  TCCR2A = 0;// set entire TCCR0A register to 0
  TCCR2B = 0;// same for TCCR0B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 1000hz increments
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
  sampling = true;            // start sampling as soon as it starts up
  overflow_1 = 0;
  setDivideFactor();
  
// RF setup
   // Initialise the IO and ISR
    vw_set_tx_pin(RFTRANSMIT);
    vw_set_rx_pin(RFRECEIVE);
    vw_set_ptt_pin(TRANSMIT_EN_PIN);
    //vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2400);	 // Bits per sec
    
    vw_rx_start();
  
  Serial.print("free memory: ");
  Serial.println(freeMemory());
}

void loop() {
  // check if the docking station is trying to sync
    sensorRecv.checkRF();
    if(sensorRecv.transmission_complete) {
      
      if(sensorRecv.PDC_sync) {
        Serial.println("syncing");
        DTDsync();
      }
      sensorRecv.printTransmission();
      sensorRecv.resetVariables();
    }
    
    // if the button has been pressed, send all collected data
    if(sync_button.pressed) {
      save_times();
      sync_button.pressed = false;
      sampling = false;
      sendData();
//      testRF();
    }
    
    // every DATA_INTERVAL seconds/100 sample the selected sensor
    if(sampling && !sensorRecv.RF_busy && overflow_1 >= DATA_INTERVAL-1) {   
        overflow_1 = 0;
        time_stamps[index] = millis()/ DIVIDE_FACTOR;
        
        // choose which sensor code to run depending on the sensor type
        switch(SENSOR_TYPE) {
          case 1:
            // BEGIN LOUDNESS SENSOR CODE
            data_val = analogRead(SENSORPIN);
            Serial.println(data_val);
            break;
          // BEGIN ULTRASOUND SENSOR CODE
          case 2:
            pinMode(SENSORPIN, OUTPUT);
            digitalWrite(SENSORPIN, LOW);
            delayMicroseconds(2);
            digitalWrite(SENSORPIN, HIGH);
            delayMicroseconds(5);
            digitalWrite(SENSORPIN, LOW);
          
            // The same pin is used to read the signal from the PING))): a HIGH
            // pulse whose duration is the time (in microseconds) from the sending
            // of the ping to the reception of its echo off of an object.
            pinMode(SENSORPIN, INPUT);
            duration = pulseIn(SENSORPIN, HIGH);
          
            // convert the time into a distance, eventually adda way to choose the 
            // measuring system (in. or cm)
    //        inches = microsecondsToInches(duration);
            if(duration > 37000)
              cm = -1;
            else
              data_val = microsecondsToCentimeters(duration);
            
    //        Serial.print(inches);
    //        Serial.println("in, ");
            Serial.print(data_val);
            Serial.println("cm");
            break;
        }

        storage[index] = data_val;
        index++;
        
        
      }

    // if the arrays are running out of room store the collected values in EEPROM
    if(index == ARR_SIZE-1) {
      save_times();
    }
}

// save the collected data into eeprom
void save_times() {
  for(int i=0;i<index;i++) {
    // stop sampling if eeprom is full
      if(eeprom_time_index == 250) {
        sampling = false;
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
//  long start_send = micros();
//  Serial.print((micros()-start_send)/1000); Serial.println(" millis to create array for first 30");
  unsigned int data_time_temp[2][MAXPAIRS] = {};
  int temp_index = 1;
  int trans_id = ULTSND_TRANS_ID; // gets changed to 0 after the first partial array is sent
                           // so the rest appear as a continuation of the entire array
                           
  Serial.print("irsync, index: "); Serial.println(eeprom_time_index);
  data_time_temp[0][0] = 0;
  data_time_temp[1][0] = DIVIDE_FACTOR;
  for(int i=0;i<eeprom_time_index;i++) {
    if(temp_index == MAXPAIRS) {
      sensorSend.createArray(trans_id, data_time_temp, temp_index);
      sensorSend.sendCondensedArray(false);      // false tells the library not to send the EoT character
//      sensorSend.printTransmission();
      trans_id = 0;
      temp_index = 0;
    }
    data_time_temp[0][temp_index] = word(EEPROM.read(2*i), EEPROM.read(2*i+1));
//    Serial.print("time: "); Serial.print(data_time_temp[0][temp_index]);
    data_time_temp[1][temp_index] = word(EEPROM.read(2*(i+STORAGE_START)), EEPROM.read(1+2*(i+STORAGE_START)));
//    Serial.print(", data value: "); Serial.println(data_time_temp[1][temp_index]);
    temp_index++;
  }
  sensorSend.createArray(trans_id, data_time_temp, temp_index);
//  sensorSend.printTransmission();
  sensorSend.sendCondensedArray();
  Serial.println("done sending");
  waitForConfirm();
}

// DTDsync is called when the docking station initiates a sync with the sensor board
void DTDsync() {
  unsigned int incoming_write[2][10] = {};
  bool quit = false;
  sensorSend.sendSyncCode();
  sensorRecv.resetVariables();
  while(!quit) {
    sensorRecv.checkRF();
    if(sensorRecv.transmission_complete) {
      // there was an error so ask for a resend
      if(!sensorRecv.parseTransmission(incoming_write)) {
        Serial.println("sending resend code");
        sensorSend.sendCommand(810);
      } else {
        interpretCommand(sensorRecv.transmission_id, incoming_write, &quit);
      }
      sensorRecv.resetVariables();
      memset(incoming_write, 0, sizeof(incoming_write));
    }
  }
}

// if a legitimate transmission has been received while in sync mode, take some action based
// on its content
void interpretCommand(int incoming_id, unsigned int incoming_write[][10], bool *quit) {
  short command_cat = incoming_id/100;
  Serial.print("interpret command with "); Serial.println(incoming_id);
 
  // interpret the command, take action, and reply to the DTD. when sendCommand includes true
  // that means it's sending a confirmation of whatever event just took place
  switch(command_cat) {
    
    // all commands that involve changing things on the sensor board are case 3
    case 3:
      if(incoming_id == 311) {
        Serial.println("setting product id");
        // product id is always stored in address 900 and 901
        loadNewInfo(incoming_write[0][0], ID_ADRS);
        sensorSend.sendCommand(incoming_id, true);
      } else if(incoming_id == 312) {
        Serial.println("setting sensor type");
        // sensorType is stored in address 902 and 903
        loadNewInfo(incoming_write[0][0], SENS_ADRS);
        sensorSend.sendCommand(incoming_id, true);
      } else if(incoming_id == 313) {
        Serial.println("setting sampling rate");
        loadNewInfo(incoming_write[0][0], DATA_INT_ADRS);
        sensorSend.sendCommand(incoming_id, true);
      }
      break;
    // for now only contains the quit command
    case 9:
//      if(incoming_id == 911) {      // uncomment when/ if there are more commands to consider
        *quit = true;
        break;
//      }
  }
        
}

// loadSensorType takes an value stored in [0][0] of incoming_write and translates it to
// a sensor type. Depending on the sensor type, the code in loop() will undertake
// the appropriate sampling procedure

//void loadSensorType(int type) {
//  
//}

// wait for the DTD to either request a resend or confirm that codes were received (this
// step should be automated by the web interface)
// if not code is received this function will return to loop without resetting storage
void waitForConfirm() {
  unsigned int incoming_write[2][10] = {};
  // check if the DTD wants a resend of the codes, otherwise clear the stored data
//  irrecv.enableIRIn();
  overflow_1 = 0;
  // wait 15 seconds
  while(overflow_1 < 1500) {
//    sensorRecv.checkIR(irrecv, results);
    
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

// loadOldInfo handles loading saved settings at the start of the program

void loadOldInfo(int address) {
  bool good_val = EEPROM.read(address++);
  Serial.print(address-1); Serial.print(", "); Serial.println(EEPROM.read(address-1));
  
   // a value of true will be saved in this address if the value has been set
  if(address == ID_ADRS+1) {
    PRODUCT_ID = word(EEPROM.read(address++), EEPROM.read(address));
    if(!good_val || PRODUCT_ID/27300 != 1) {
      PRODUCT_ID = 27300;
    }
    sensorSend.my_id = PRODUCT_ID;
    Serial.println("using product id: "); Serial.println(PRODUCT_ID);
  } else if(address == SENS_ADRS+1) {
    SENSOR_TYPE = word(EEPROM.read(address++), EEPROM.read(address));
    
    // change this as we add more sensors
    if(!good_val || SENSOR_TYPE > 2) {
      SENSOR_TYPE = 1;
    }
    Serial.println("using sensor: "); Serial.println(SENSOR_TYPE);
  } else if(address == DATA_INT_ADRS+1) {
    DATA_INTERVAL = word(EEPROM.read(address++), EEPROM.read(address));

    if(!good_val || (DATA_INTERVAL < 10 || DATA_INTERVAL > 60000)) {
      DATA_INTERVAL = 1000;
    }
    Serial.println("using sample rate: "); Serial.println(DATA_INTERVAL);
  }
}

// LoadNewInfo handles loading new product information and settings into the sensor's memory

void loadNewInfo(unsigned int new_info, int address) {
  Serial.print("saving id "); Serial.println(new_info);
//  if(address == ID_ADRS) {
//    PRODUCT_ID = new_info;
//    sensorSend.my_id = new_info;
//  } else if(address == SENS_ADRS) {
//    SENSOR_TYPE = new_info;
//  } else if(address = DATA_INT_ADRS) {
//    DATA_INTERVAL = new_info;
//  }
  EEPROM.write(address++, true);
  Serial.print(address-1); Serial.print(", "); Serial.println(EEPROM.read(address-1));
  EEPROM.write(address++, highByte(new_info));
  EEPROM.write(address, lowByte(new_info));
  // make sure all settings are up to date
  loadOldInfo(address-2);
}

//the interrupt service routine. Calls the rise function in the button1 class and tells
// the teensy to send new values next time through loop
void rise_funct()
{
//  Serial.println("button pressed");
  sync_button.rise();
}

ISR(TIMER2_COMPA_vect) {
  overflow_1++;
  if(vw_have_message()) {
    sampling = false;
  }
}

void setDivideFactor() {
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
//  sampling = true;    // uncomment this to continue sampling after sending data
  overflow_1 = 0;
}

void testRF() {
  Serial.println("testing RF");
  char msg[27] = {'a','b','c','d','e',
            'f','g','h','i','j','k','l',
            'm','n','o','p','q','r','s',
            't','u','v','w','x','y','z',
            '0'};


    //msg[6] = count;
//    digitalWrite(led_pin, HIGH); // Flash a light to show transmitting
    vw_send((uint8_t *)msg, 26);
    vw_wait_tx(); // Wait until the whole message is gone
//    digitalWrite(led_pin, LOW);
//    delay(1000);
}
