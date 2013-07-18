#include <PDCsend.h>
/*
By: Marc Bucchieri
Contributions: http://www.gammon.com.au/forum/?id=10892&reply=1#reply1 for SPI code

Code for Teensy to act as SPI master for an Arduino slave that controls the Physical
Design Compass screen and timer. 
*/  

#include <SPI.h>
#include "button.h"
//#include "PDCsend_v02.h"
#include <IRremote.h>


// Write HIGH or LOW to this pin to signal to Arduino when SPI transfer is about to occur
const int SSPIN = 0;
const int RECEIVEPIN = 9;
const long PRODUCT_ID = 73201; //"=> PDC01"
const int SEND_TIMES_ID = 001;
boolean send_info;
boolean in_timer;
int old_case;
int timeOfLast;
button button_1(0);
button button_2(1); 
const int sensorPin = A0;
unsigned long time_1sectionTime[15];
unsigned long test_arr[] = {1,2,3,4,5,6,7,8};

// new instance of PDCsend class to send times using IR
PDCsend myPDC;

// tryna get some IR
IRrecv irrecv(RECEIVEPIN);
decode_results results;

void setup() {
  Serial.begin(9600);
//  PDCsend myPDC;
  Serial.println("made it to setup!");
//  myPDC.createArray(73201, 1, test_arr);
//  Serial.println("done creating array");
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  pinMode(SSPIN, OUTPUT);
  in_timer = true;
  
  old_case = -1;
  send_info = false;
  attachInterrupt(button_1.interrupt_pin, rise1_funct, RISING);
  attachInterrupt(button_2.interrupt_pin, rise2_funct, RISING);
  pinMode(sensorPin, INPUT);
  
  // PDC on the arduino will start on the load screen, which allows the user to 
  // select or clear memory slots
  runLoadScreen();
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
          Serial.println("sending times");
          sendSPI();
          sendTimes();
          for(int i=0;i<8;i++) {
            Serial.print(time_1sectionTime[i]); Serial.print(", ");
          }
          Serial.println();
          break;
          
        case 2:
          Serial.println("drawing graph");
          button_1.pressed = true;
          sendSPI();
          button_1.pressed = false;
          while(!button_1.pressed) {
            // wait for them to quit the graph
          }
          sendSPI();
          button_1.pressed = false;
          break;
      }
      in_timer = true;
      break;
    }
  }
  Serial.println("returning from runMenu");
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

int sensorValue() 
{
   return map(analogRead(sensorPin), 0, 1023, 0, 24); 
}

 //maps the analog read output to a new number
int caseSelect(int sensorValue, int upper)
{
  return(map(sensorValue, 0, 23, 0, upper-1)); 
} 

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
    myPDC.createArray(PRODUCT_ID, SEND_TIMES_ID, time_1sectionTime);      // myPDC will put all the times and categores in
    myPDC.printTransmission();
    myPDC.sendArray();                          // a 2D array and send it to the docking station
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
  
