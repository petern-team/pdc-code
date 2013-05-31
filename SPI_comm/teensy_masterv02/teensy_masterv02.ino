/*
Teensy-Arduino SPI communication, prototype v01

teensy master code
*/

#include <SPI.h>
#include "button.h"
#include "PDCsend_v02.h"
#include <IRremote.h>

const int SSPIN = 0;
//boolean button1_pressed;
//boolean button2_pressed;
//int potenti_state;
boolean send_info;
boolean in_timer;
int old_case;
int timeOfLast;
button button_1(0);
button button_2(1); 
const int sensorPin = A0;
//PDCsend myPDC;


void setup() {
  Serial.begin(9600);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  pinMode(SSPIN, OUTPUT);
  timeOfLast = -200;
  in_timer = true;
  
  old_case = -1;
  send_info = false;
  attachInterrupt(button_1.interrupt_pin, rise1_funct, RISING);
  attachInterrupt(button_2.interrupt_pin, rise2_funct, RISING);
  pinMode(sensorPin, INPUT);
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

void runMenu() 
{
 // boolean quit = false;
  while(!in_timer) {
    if(sensorValue() != old_case) {
      sendSPI();
      old_case = sensorValue();
    }
    
    if(button_2.pressed) {
      Serial.println("button 2 was pressed in menu");
      sendSPI();
      button_2.pressed = false;
      in_timer = true;
      break;
    }
    
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
//          myPDC.createArray(time_1.sectionTime);      // myPDC will put all the times and categores in
//          myPDC.sendArray();                          // a 2D array and send it to the docking station
          button_1.pressed = false;
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

void rise1_funct(){//the interrupt service routine. Calls the rise function in the button1 class. Handles the pause button
  noInterrupts();
  button_1.rise();
  send_info = true;  
  interrupts();
}

void rise2_funct(){//the interrupt service routine. Calls the rise function in the button1 class. Handles the pause button
  noInterrupts();
  button_2.rise(); 
  send_info = true; 
  interrupts();
}

int sensorValue() 
{
   return map(analogRead(sensorPin), 0, 1023, 0, 24); 
}

 //maps the analog read output to a new number
int caseSelect(int sensorValue, int upper)
{
  return(map(sensorValue, 0, 23, 0, upper-1)); 
} 


void sendSPI()
{
 // if(millis()-timeOfLast > 200) {
    char output[] = {button_1.pressed,button_2.pressed,sensorValue(),-1};
    digitalWrite(SSPIN, LOW);
    
    for(int i=0;i<4;i++) {
      Serial.println(output[i]+0);
      SPI.transfer(output[i]);
    }
    
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
   // timeOfLast = millis();
 // }
}
