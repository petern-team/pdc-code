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
//PDCsend myPDC;


const int NUM_STEPS=8;// Number of steps in the design process
const int MAX_STEPS=12; 
//const int sensorPin = A5; //the pin on the Arduino that the middle (wiper) pin of the potentiometer is connected to
//const int READ_MAX=1023; // the maximum value analogRead can have, used to map the range of values from the potentiometer
timer time_1; // make a new instance of the timer struct
//button button_1(0); //make a new instance of the button class
//button button_2(1); 
int caseValue=0;
unsigned int eeprom_write_last=0;
int slot=0; 
String step_list[MAX_STEPS] = {"Define     Problem","Research","Brainstorm","Select",
              "Construct","Test","Communicate","Redesign","","","",""};
int old_case; 
volatile boolean button_1pressed;
volatile boolean button_2pressed;
volatile int sensorValue;
              
//SPI library and variables ----------------------------------------------------------

#include <SPI.h>

int SPI_in[100];
volatile byte pos;
volatile boolean SPI_complete;


// SPI interrupt routine
// information will be received in triplets of button_1pressed, button_2pressed,
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

//--------------------------------------------------------------------------------

void setup() {
  // all of the following is SPI stuff-------------------------------------------------
  SPI.attachInterrupt();
  pinMode(MISO, OUTPUT);
 // pinMode(13, OUTPUT);
  
  SPCR |= _BV(SPE);
  
  pos=0;
  SPI_complete = false;
  
  Serial.begin(9600); 
  GLCD.Init(); // start the GLCD code
  GLCD.SelectFont(Arial_14);
  GLCD.CursorTo(0,0); 
  load_screen();
  
  old_case = -1;

}


void loop() {
  
  //SPI information upload
  checkSPI();
  
  //------------------------ BEGIN DESIGN COMPASS CODE --------------------------------

if (button_2pressed) { 
    button_1pressed = false;    // reset both buttons
    button_2pressed=false;
    int menuSelect = displayMenu();
    Serial.print("menu returned: "); Serial.println(menuSelect);
    GLCD.ClearScreen();
    
    switch (menuSelect) {
      case 0:
        Serial.println("load screen");
        load_screen();
        break;
      case 1:
        Serial.println("send times");
//        myPDC.createArray(time_1.sectionTime);    // these two cause memory problems
//        myPDC.sendArray();
        break;
      case 2:
        Serial.println("draw graph");
        draw();
        break;
      case 3:
        break;
      }
  } 
    
    caseValue = caseSelect(sensorValue, NUM_STEPS);    // the current category gets whatever the dial is pointing to
    String section=step_list[caseValue];
    
    //logic used to avoid having to clear the screen every iterartion.
    //allows the arduino to remember what the last section it timed was. 
    if (caseValue != old_case) {
      GLCD.ClearScreen();
      old_case=caseValue;
    }
    
    // this is the main interface, with the section and the current time for that section. 
    GLCD.CursorTo(0,0);
    GLCD.print(section);
    GLCD.SelectFont(fixednums8x16);
    GLCD.CursorTo(0,2);
    int hrs=time_1.sectionTime[caseValue]/3600;
    int mins=(time_1.sectionTime[caseValue]%3600)/60;
    int secs=time_1.sectionTime[caseValue]%60;
    GLCD.Printf(("%02d:%02d:%02d"), hrs, mins, secs);
    GLCD.SelectFont(Arial_14);
    
    GLCD.GotoXY(0,50);
    GLCD.print("Menu");
    if(button_1pressed) {
      GLCD.GotoXY(87,50);
      GLCD.print("Pause");
    } else {
      GLCD.GotoXY(87,50);
      GLCD.print("     Start     ");
    }
    logTime_1();   
 
}

int displayMenu()
{
  String menuList[] = {"Change    Memory    Slot","Send    Data","Print    Graph"};
  boolean quit=false;
  int old_val = -1;
  
  GLCD.ClearScreen();
  GLCD.SelectFont(Arial_14);
  GLCD.GotoXY(85,50);
  GLCD.print("Select");
  GLCD.GotoXY(0,50);
  GLCD.print("Back");
  
  while(!quit) {
  //  Serial.println(SPI_complete);
    checkSPI();
    int menuValue = caseSelect(sensorValue, 3);
    if(menuValue == 3)      // do this to deal with a faulty potentiometer
      menuValue = 2;

    if (menuValue != old_val) {
      old_val = menuValue;      
      for(int i=0;i<3;i++) {
        Serial.print(i); Serial.print(": "); Serial.println(menuList[i]);
        GLCD.CursorTo(0,i);
        if(menuValue == i) {
          GLCD.SetFontColor(WHITE);
        } else {
          GLCD.SetFontColor(BLACK);
        }
        GLCD.print(menuList[i]);
      }   
    }
    
    if(button_2pressed) {
      button_2pressed = false;
      return 3;
    }
    
    if(button_1pressed) {
      button_1pressed = false;
      return menuValue;
    }
  } 
}

// check whether the timer should be running or paused, based on the state of button 1
// write the appropriate times to the sectionTime array
void logTime_1()
{ 
  if (button_1pressed==true){                          // pressed means the timer is running
    time_1.time_val = millis()/ 1000;                   //get the current time
    time_1.storedTime = time_1.sectionTime[caseValue];  // store the time that was previously in that section
    time_1.sectionTime[caseValue] = time_1.storedTime + (time_1.time_val - time_1.lastTime);
    
    //add the time elapsed in this section to the proper array element. 
    if(time_1.time_val-eeprom_write_last>10){
       save_times();
       eeprom_write_last=millis()/1000; 
     } 
     
   // this next line is probably unnecessary
    time_1.lastTime = time_1.time_val; //update the lastTime field with the time that this function started   
     if (time_1.lastSection != caseValue) {
        if (time_1.time_val-time_1.last_section_time>5) {
           time_1.steps[time_1.count]=caseValue;
           time_1.count++; 
         }
         time_1.lastSection = caseValue; //update the lastSection field with the section used in this function (not used for anything yet)
      }
    
    GLCD.SelectFont(Arial_14);
    return; 
  }
  time_1.lastTime=millis()/1000; 
}

//The algorithm to save the time values into EEPROM memory. Only works with short ints now.
// the function takes the high and low bytes of each stored integer, and then saves them sequentially 
// in EEPROM memory
void save_times()
{ 
    for(int i=0; i<NUM_STEPS; i++){
	byte hibyte=highByte(time_1.sectionTime[i]);
	byte lobyte=lowByte(time_1.sectionTime[i]);
	EEPROM.write((2*i)+(slot*(NUM_STEPS)), hibyte);
	EEPROM.write(2*i+(slot*(NUM_STEPS))+1, lobyte);
    }
 }
 
//  this function takes each of the sequentially written bytes and converts them into 
//  an interger via the word function. 
//  this is used to load the saved time values.    <- More so this
void load_times(){ 
  for(int i=0; i<NUM_STEPS;  i++){
    //if (i<NUM_STEPS){
      time_1.sectionTime[i]=word(EEPROM.read(2*i+(slot*(NUM_STEPS))), EEPROM.read(2*i+(slot*(NUM_STEPS)+1)));
   // }
 /* else{
     time_1.steps[2*i-NUM_STEPS]=EEPROM.read(2*NUM_STEPS+i-NUM_STEPS+slot*(NUM_STEPS+50)); 
    }
    */
  }
  
}

// This function is called from the menu, and draws a bar graph of the time spent so far
// divided by category
void draw()
{
  int heights[MAX_STEPS];
  for (int i=0; i<MAX_STEPS; i++){
    heights[i]=0;
  }
  
  GLCD.SelectFont(System5x7); 
  GLCD.ClearScreen();
  get_heights(heights); //gets the heights for each column
  GLCD.DrawHLine(0,54, 76); //draws the boundary lines for the graph
  GLCD.DrawVLine(76,0,63);
  int sum=sum_times();         // seems to be completely unnecessary
  GLCD.GotoXY(78, 24);
  int hrs=sum/3600;
  int mins=(sum%3600)/60;
  int secs=sum%60;
  GLCD.Printf(("%02d:%02d:%02d"), hrs, mins, secs);
  GLCD.GotoXY(78, 4);
  GLCD.print("Total"); 
  GLCD.GotoXY(78,13);
  GLCD.print("Time:"); 
   
  for(int i=0; i<NUM_STEPS; i++){
    GLCD.GotoXY(10*i, 55);
    GLCD.print(i+1); //prints the step numbers to the bottom of the graph
    if (heights[i]!=0){
      GLCD.DrawRect(10*i,54-heights[i], 5, heights[i]); //draws the frame of the rectangle
       // the draw function takes the upper left corner and builds downwards. 
      GLCD.FillRect(10*i,54-heights[i], 5, heights[i]); //fills the rectangle
      }
  }
  
  GLCD.SelectFont(Arial_14);
  GLCD.GotoXY(95,50);
  GLCD.print("Back");
    while(button_1pressed==false){
      checkSPI();
      delay(25); //waste time, to keep the ardino from using too much power
    } 
   button_1pressed = false;
   GLCD.ClearScreen(); 
}

// Find the height that each bar of the graph should be
void get_heights(int heights[])
{
  int total_time=sum_times(); 
  for (int i=0; i<NUM_STEPS; i++){
    heights[i]=map(floor(time_1.sectionTime[i]), 0, total_time, 0, 40);
    }
}

// This function is called during setup. It allows the user to choose a memory slot for the times to 
// be stored in and gives the user a chance to clear any of the memory slots
void load_screen()
{
  Serial.println("load screen called");
  int sensor_val=0;
  int old_cursor=0; 
  boolean quit = false;
    GLCD.GotoXY(85,50);
    GLCD.print("Select");
    GLCD.GotoXY(0,50);
    GLCD.print("Clear");
    while (!quit){
	checkSPI();
	slot=caseSelect(sensorValue,9);
	if (slot!=old_cursor) //logic used to avoid having to clear the screen every iterartion.
        //allows the arduino to remember what the last cursor number it timed was. 
        {       
           GLCD.ClearScreen();
           old_cursor=slot;
           GLCD.GotoXY(85,50);
           GLCD.print("Select");
           GLCD.GotoXY(0,50);
           
           GLCD.print("Clear");
           
        }
        GLCD.CursorTo(0,0);
        GLCD.print("Memory    Slot     "); GLCD.print(slot+1);
        
        //If they choose select, load the times already in that memory slot and end the function
        if (button_1pressed) {
          button_1pressed=false;
          load_times(); 
          break; 
        }
        
        // If they choose clear, call the clear_slot function and continue with the loop
        if (button_2pressed) {
          button_2pressed=false;
          clear_slot(); 
          button_1pressed = false;
          button_2pressed = false;
          GLCD.ClearScreen();
          old_cursor = -1;
        } 
     }  
}
  
// Ask user to confirm, then  
// overwrite the selected slot of the EEPROM with zeroes.   
void clear_slot()
{ 
  boolean quit = false;
  GLCD.ClearScreen();
  GLCD.CursorTo(0,0);
  GLCD.print("Clear    Memory    Slot    "); GLCD.print(slot+1); GLCD.print("?");
  while(!quit) {  
    checkSPI();
    GLCD.CursorTo(1,0);
    GLCD.GotoXY(85,50);
    GLCD.print("No");
    GLCD.GotoXY(0,50);
    GLCD.print("Yes");

    if (button_1pressed) {
      break;
    }
    
    if (button_2pressed) {
      for(int i=0; i<(NUM_STEPS*2); i++){
         EEPROM.write(i+(slot*(NUM_STEPS)), 0);
      }
      GLCD.ClearScreen();
      GLCD.CursorTo(3,2);
      GLCD.print("Slot    "); GLCD.print(slot+1); GLCD.print("    cleared.");
      delay(3000);
      break;
    }  
  }
}


// Add up all the times from the sectionTime array
int sum_times(){
  int sum=0;
  for (int i=0; i<NUM_STEPS; i++){
    sum+=time_1.sectionTime[i];
  }
   return sum; 
}


//--------------------------------------- UTILITIES -----------------------------------
  
void checkSPI()
{    
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
}
  
 //maps the analog read output to a new number
int caseSelect(int sensorValue, int upper)
{
  return(map(sensorValue, 0, 23, 0, upper-1)); 
} 

