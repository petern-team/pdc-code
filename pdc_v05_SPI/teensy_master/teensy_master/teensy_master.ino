
/*
By: Marc Bucchieri
Contributions: http://www.gammon.com.au/forum/?id=10892&reply=1#reply1 for SPI code

Code for Teensy to act as SPI master for an Arduino slave that controls the Physical
Design Compass screen and timer. 
*/  

#include <SPI.h>
#include "button.h"
#include <EEPROM.h>

#include <IRremote.h>
#include <PDCsend.h>
#include <PDCreceive.h>


// Write HIGH or LOW to this pin to signal to Arduino when SPI transfer is about to occur
const int SSPIN = 0;
const int RECEIVEPIN = 8;
const int SEND_TIMES_ID = 501;  // id used when sending times to the DTD
const int NUM_CATS = 8;

unsigned int PRODUCT_ID = 63201; //"=> PDC01", not a constant but gets loaded every time from eeprom
boolean send_info;
boolean in_timer;
int old_case;
int timeOfLast;
button button_1(0);
button button_2(1); 
const int sensorPin = A0;
unsigned int time_1sectionTime[15];
unsigned long test_arr[] = {1,2,3,4,5,6,7,8};

// new instance of PDCsend class to send times using IR
PDCsend pdcSend;
PDCreceive pdcReceive;

// tryna get some IR
IRrecv irrecv(RECEIVEPIN);
decode_results results;

void setup() {
  Serial.begin(9600);
//  PDCsend pdcSend;
  Serial.println("made it to setup!");
//  pdcSend.createArray(73201, 1, test_arr);
//  Serial.println("done creating array");
//  pinMode(10, OUTPUT);
//  digitalWrite(10, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  pinMode(SSPIN, OUTPUT);
  in_timer = true;
  old_case = -1;
  send_info = false;
  
  irrecv.enableIRIn();
  pinMode(sensorPin, INPUT);
  
  attachInterrupt(button_1.interrupt_pin, rise1_funct, RISING);
  attachInterrupt(button_2.interrupt_pin, rise2_funct, RISING);
  
  // PDC on the arduino will start on the load screen, which allows the user to 
  // select or clear memory slots
  loadOldId();
  pdcSend.my_id = PRODUCT_ID;
  runLoadScreen();
//  pdcSend.createArray(SEND_TIMES_ID, time_1sectionTime);      // pdcSend will put all the times and categores in
//    pdcSend.printTransmission();
//    pdcSend.sendArray();                          // a 2D array and send it to the docking station
}

void loop() 
{
  if(sensorValue() != old_case) {
    send_info = true;
    old_case = sensorValue();
  }
  
  if(send_info) {
     sendSPI();
  }
}

// This function is called whenever the user presses button 2 from the timer, meaning
// that they want the menu to be displayed. The function is necessary because button 
// presses have different meanings from the menu

void runMenu() 
{
  while(!in_timer) {
    
    // check if the potentiometer position has changed
    if(sensorValue() != old_case) {
      sendSPI();
      old_case = sensorValue();
    }
    
    // user pressed back from the menu
    if(button_2.pressed) {
//      Serial.println("button 2 was pressed in menu");
      sendSPI();
      button_2.pressed = false;
      in_timer = true;
      break;
    }
    
    // choose an action based on the position of the potentiometer
    if(button_1.pressed) {
      Serial.println("button 1 was pressed in menu");
      int menuValue = caseSelect(sensorValue(), 3);
      Serial.println(menuValue);
      
      switch (menuValue) {
        
        case 0:
          Serial.println("calling load_screen");
          sendSPI();
          button_1.pressed = false;
          runLoadScreen();
          break;
          
        case 1:
          Serial.println("computer sync menu");
          sendSPI();
          runSyncScreen();
//          sendSPI();
//          sendTimes();
//          for(int i=0;i<NUM_CATS;i++) {
//            Serial.print(time_1sectionTime[i]); Serial.print(", ");
//          }
//          Serial.println();
          break;
          
        case 2:
          Serial.println("drawing graph");
          button_1.pressed = true;
          sendSPI();
          button_1.pressed = false;
          while(!button_1.pressed) {
            // wait for them to quit the graph
          }
//          sendSPI();
//          button_1.pressed = false;
          break;
      }
      in_timer = true;
      button_2.pressed = false;
      button_1.pressed = false;
      sendSPI();
      break;
    }
  }
  Serial.println("returning from runMenu");
}

// runSyncScreen will handle Dtd <-> PDC interaction, beginning by sending a message with the 
// product ID which the DtD will return if it receives it

void runSyncScreen() {
  boolean quit = false;
  Serial.println("run sync screen");
  unsigned int incoming_write[2][10];
  for(int i=0;i<10;i++) {
    incoming_write[0][i] = 0;
    incoming_write[1][i] = 0;
  }

  pdcSend.sendSyncCode(PRODUCT_ID);
  irrecv.enableIRIn();
//  for(int i=0;(i<4 && !pdcReceive.PDC_sync);i++) {
    for(int j=0;(j<150 && !pdcReceive.PDC_sync);j++) {
      pdcReceive.checkIR(irrecv, results);
      delay(20);
    }
    
//  }
  
  if(!pdcReceive.PDC_sync) {
    Serial.println("No DTD found");
    sendSPIdata(31);
    return;
  } else {
    Serial.println("found it!");
    sendSPIdata(30);
  }
  pdcReceive.resetVariables();
  
  Serial.println("waiting for next transmission...");
  // goes here if sync was successful
  while(!quit) {
    pdcReceive.checkIR(irrecv, results);
    if(pdcReceive.transmission_complete) {
      if(!pdcReceive.parseTransmission(incoming_write)) {
        sendSPIdata(91);
        // if no error, call interpretCommand, if the function returns false exit sync mode
      } else if(!interpretCommand(pdcReceive.transmission_id, incoming_write)) {
        pdcReceive.resetVariables();
        return;
      }
//      pdcReceive.printTransmission();
      pdcReceive.resetVariables();
//      irrecv.enableIRIn();
      Serial.println("waiting for next transmission...");
//      pdcReceive.parseTransmission();
    }
  }

  Serial.println("leaving runsyncscreen");
}

// The load screen is displayed when the PDC is turned on and when the user selects
// "Change Memory Slot" from the menu. From the load screen, the user can either select
// a memory slot to start timing in, or can clear any of the memory slots

void runLoadScreen()
{
  boolean in_load_screen = true;
  while(in_load_screen) {
    if(sensorValue() != old_case) {
      sendSPI();
      old_case = sensorValue();
    }
    
    if(button_1.pressed) {
      Serial.println("exiting load screen");
      sendSPI();
      button_1.pressed = false;
      break;
    }
    if(button_2.pressed) {
      Serial.println("calling clear screen");
      sendSPI();
      button_2.pressed = false;
      boolean in_clear_screen = true;
      while(in_clear_screen) {
        if(button_1.pressed || button_2.pressed) {
          sendSPI();
          button_1.pressed = false;
          button_2.pressed = false;
          break;
        }
      }
    }
  }
}

//-------------------------------------- UTILITIES ---------------------------------
// loadOldId loads the product id stored in eeprom. if no id exists, this function automatically assigns
// the pdc id 63200
void loadOldId() {
  PRODUCT_ID = word(EEPROM.read(0), EEPROM.read(1));
  Serial.print("found product id: "); Serial.println(PRODUCT_ID);
    if(PRODUCT_ID/63200 != 1) {
    Serial.println("no product id stored: using id 63200");
    PRODUCT_ID = 63200;
  }
}


//the interrupt service routine. Calls the rise function in the button1 class and tells
// the teensy to send new values next time through loop
void rise1_funct()
{
  noInterrupts();
  button_1.rise();
  send_info = true;  
  interrupts();
}

//the interrupt service routine. Calls the rise function in the button2 class and tells
// the teensy to send new values next time through loop
void rise2_funct(){
  noInterrupts();
  button_2.rise(); 
  send_info = true; 
  interrupts();
}


// check the potentiometer position and map to a number between 0 and 24 so that it can
// be sent as a byte

// CHECK CALIBRATION FOR DIFFERENT POTS

int sensorValue() 
{
   return map(analogRead(sensorPin), 0, 910, 0, 24); 
}

 //maps the analog read output to a new number
int caseSelect(int sensorValue, int upper)
{
  return(map(sensorValue, 0, 23, 0, upper-1)); 
} 

// interpretCommand returns true if the PDC should stay in sync mode and false if it should quit
// otherwise the function handles all incoming commands from the DTD

boolean interpretCommand(int incoming_id, unsigned int incoming_write[][10]) {
  short command_cat = incoming_id/100;
  switch(command_cat) {
    // case 1 is the category of query commands
    case 1:
      if(incoming_id == 101) {
        sendSPIdata(41);
//        sendSPI();
          sendTimes();
//          for(int i=0;i<NUM_CATS;i++) {
//            Serial.print(time_1sectionTime[i]); Serial.print(", ");
//          }
//          Serial.println();
      }
      break;
    // case 3 is the category for write commands
    case 3:
      if(incoming_id == 301) {
        Serial.println("301 received");
        sendSPIdata(51);
        loadTimes(incoming_write);
        
        // this will be handled by the computer
        pdcSend.sendConfirm(PRODUCT_ID, incoming_id);
      } else if(incoming_id == 302) {
        Serial.println("loading new id");
        sendSPIdata(52);
        loadProductId(incoming_write);
        pdcSend.sendConfirm(PRODUCT_ID, incoming_id);
      }
      break;
    // for now case 9 only contains the quit command
    case 9:
      if(incoming_id == 901) {
        sendSPIdata(32);
        return false;
      }
      break;
  }
  return true;  
}

// loadTimes will be called when the DTD is loading times externally onto the design compass
// at first the times will go into the current memory slot, but this can be adjusted later

void loadTimes(unsigned int incoming_write[][10]) {
  Serial.println("loading times into SPI array");
  char SPI_times[3][NUM_CATS];
  byte the_byte;
  for(int i=0;i<NUM_CATS;i++) {
//    the_byte = 
    SPI_times[0][i] = incoming_write[0][i];
    Serial.print(SPI_times[0][i]+0); Serial.print(" ");
    the_byte = incoming_write[1][i] >> 8;      // first one of each pair will only be the 8 most significant
    SPI_times[1][i] = the_byte;
    Serial.print(SPI_times[1][i]+0);Serial.print(" ");
    the_byte = incoming_write[1][i] & B11111111;
    SPI_times[2][i] = the_byte;
    Serial.print(SPI_times[2][i]+0); Serial.print(" ");
    Serial.println();
  }
  loadSPItimes(SPI_times);
}

// loadProductId used to load a new product ID sent from the DTD
void loadProductId(unsigned int incoming_write[][10]) {
  unsigned int new_id = incoming_write[0][0];
  Serial.print("saving id "); Serial.println(new_id);
  EEPROM.write(0, highByte(new_id));
  EEPROM.write(1, lowByte(new_id));
}

//-----------------------------------------SPI COMMUNICATION-----------------------------------

// SendSPI is called whenever the values for button 1 or 2 or the potentiometer have changed.
// It sends the relevant values to Arduino and calls runMenu when appropriate

void sendSPI()
{
    char output[] = {button_1.pressed,button_2.pressed,sensorValue(),-1};
    digitalWrite(SSPIN, LOW);
    
    for(int i=0;i<4;i++) {
      Serial.print(output[i]+0); Serial.print(", ");
      SPI.transfer(output[i]);
      delay(1);
    }
    Serial.println();  
    
    digitalWrite(SSPIN, HIGH);
    send_info = false;
    
    if(button_2.pressed && in_timer) {
      // PDC is in the menu
      button_1.pressed = false;
      button_2.pressed = false;
      in_timer = false;
      runMenu();
    }
    delay(50);
}

// loadSPItimes is used specifically to load times from the DTD onto the PDC
void loadSPItimes(char SPI_times[][8]) {
//  char output[3*NUM_CATS];

  digitalWrite(SSPIN, LOW);
  for(int i=0;i<NUM_CATS;i++) {
    for(int j=0;j<3;j++) {
//      Serial.print("sending: "); Serial.println(SPI_times[j][i]+0);
      SPI.transfer(SPI_times[j][i]+0);
      delay(1);
    }
    SPI.transfer(-1);
    delay(50);
  }
  digitalWrite(SSPIN, HIGH);
}

// sendSPIdata works the same as SendSPI() but replaces the sensor Value with a code to give the
// PDC instructions. So far this is only used when syncing with the docking station

void sendSPIdata(int data)
{
    char output[] = {button_1.pressed,button_2.pressed,data,-1};
    digitalWrite(SSPIN, LOW);
    
    for(int i=0;i<4;i++) {
      Serial.print(output[i]+0); Serial.print(", ");
      SPI.transfer(output[i]);
      delay(1);
    }
    Serial.println();
    
    digitalWrite(SSPIN, HIGH);
    delay(50);
}

// If "Send Data" has been selected in the menu, get the timer values from Arduino
// and send them using the PDCsend class

void sendTimes()
{
  digitalWrite(SSPIN, LOW);
  
  getValue();
  delay(20);
  
  for(int i=0;i<8;i++) {
    time_1sectionTime[i] = getValue()+0;
//    Serial.print(time_1sectionTime[i]+0); Serial.print(", ");
    delay(20);
  }
    digitalWrite(SSPIN, HIGH);
    
    Serial.println("about to call createArray");
    pdcSend.createArray(SEND_TIMES_ID, time_1sectionTime);      // pdcSend will put all the times and categores in
    pdcSend.printTransmission();
    pdcSend.sendArray();                          // a 2D array and send it to the docking station
    irrecv.enableIRIn();
    button_1.pressed = false;
    button_2.pressed = false;
    
    // send new button and potentiometer values to Arduino
    sendSPI();
}

// Send the Arduino an empty byte and store the value it returns, which will correspond
// with the stored timer values

byte getValue()
{
  byte from_slave = SPI.transfer(0);
  delayMicroseconds(20);
  return from_slave;
}
  
