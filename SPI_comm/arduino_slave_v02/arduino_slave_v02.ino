/*
Teensy-Arduino SPI communication, prototype v01

arduino slave code
*/
/*
 Dominic Guri
 Andrew Whitehead
 Marc Bucchieri
 5/22/13
 Physical Design Compass Potentiometer Wheel Prototype
 Tufts Center for Engineering Education and Outreach
 GLCD library will be included with the code. In addition,
 it can be downloaded from http://code.google.com/p/glcd-arduino/downloads/detail?name=glcd-v3-20111205.zip
 */

#include <glcd.h>
#include <allFonts.h>
#include "timer.h" //include the timer struct
#include <EEPROM.h>
#include "PDCsend_v02.h"
#include <IRremote.h>

//// create an instance of the PDCsend class, which will handle sending times as
//// IR codes to the teensy docking stations
//PDCsend myPDC(3);
//
//
//const int NUM_STEPS=8;// Number of steps in the design process
const int MAX_STEPS=12; 
//const int sensorPin = A5; //the pin on the Arduino that the middle (wiper) pin of the potentiometer is connected to
//const int READ_MAX=1023; // the maximum value analogRead can have, used to map the range of values from the potentiometer
//int  old_case= -1;  //the default case for the DC to begin
//timer time_1; // make a new instance of the timer struct
//button button_1(0); //make a new instance of the button class
//button button_2(1); 
int caseValue=0;
//unsigned int eeprom_write_last=0;
//int slot=0; 
String step_list[MAX_STEPS] = {"Define     Problem","Research","Brainstorm","Select",
              "Construct","Test","Communicate","Redesign","","","",""};
int old_case;              
              
//SPI library and variables ----------------------------------------------------------

#include <SPI.h>

int SPI_in[100];
volatile byte pos;
volatile boolean SPI_complete;

boolean button_1pressed;
boolean button_2pressed;
int sensorValue;
//--------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600); 
//  GLCD.Init(); // start the GLCD code
//  GLCD.SelectFont(Arial_14);
//  GLCD.CursorTo(0,0); 
//  attachInterrupt(button_1.interrupt_pin, rise1_funct, RISING);
//  attachInterrupt(button_2.interrupt_pin, rise2_funct, RISING); 
//  load_screen(); 
  
  old_case = -1;
  // all of the following is SPI stuff-------------------------------------------------
  SPI.attachInterrupt();
  pinMode(MISO, OUTPUT);
  pinMode(13, OUTPUT);
  
  SPCR |= _BV(SPE);
  
  pos=0;
  SPI_complete = false;
}

// SPI interrupt routine
// information will be received in triplets of button_1.pressed, button_2.pressed,
// and sensorValue. At the end of each triplet will be a sentinel (-1)
ISR (SPI_STC_vect)
{
  char input = SPDR;
  
  // add to buffer if room
  if (pos < sizeof(SPI_in)) {
    SPI_in[pos] = input+0;
    
    // example: newline means time to process buffer
    if (SPI_in[pos] == -1)
      SPI_complete = true;
      
    pos++;
  }  else {
    Serial.println("Error: buffer full");
  }
}

void loop() {
  
  //SPI information upload
  if(SPI_complete) {
    button_1pressed = SPI_in[0];
    button_2pressed = SPI_in[1];
    sensorValue = SPI_in[2];
    for(int i=pos; i>0; i--) {
      SPI_in[--pos] = 0;
    }
    SPI_complete = false;
    Serial.print("button 1: "); Serial.println(button_1pressed);
    Serial.print("button 2: "); Serial.println(button_2pressed);
  } 
    
  caseValue = caseSelect(sensorValue, 8);
  
  if(caseValue != old_case) {
    Serial.println(step_list[caseValue]);
    old_case = caseValue;
  }  
  
  
}

//--------------------------------------- UTILITIES -----------------------------------
  
 //maps the analog read output to a new number
int caseSelect(int sensorValue, int upper)
{
  return(map(sensorValue, 0, 23, 0, upper-1)); 
} 

