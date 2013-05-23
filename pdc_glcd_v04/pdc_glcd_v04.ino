//#include <SPI.h>

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
#include "button.h" //include the button class
#include <EEPROM.h>
#include "PDCsend_v02.h"
#include <IRremote.h>

// create an instance of the PDCsend class, which will handle sending times as
// IR codes to the teensy docking stations
PDCsend myPDC(3);


const int NUM_STEPS=8;// Number of steps in the design process
const int MAX_STEPS=12; 
const int sensorPin = A5; //the pin on the Arduino that the middle (wiper) pin of the potentiometer is connected to
const int READ_MAX=1023; // the maximum value analogRead can have, used to map the range of values from the potentiometer
int  old_case= -1;  //the default case for the DC to begin
timer time_1; // make a new instance of the timer struct
button button_1(0); //make a new instance of the button class
button button_2(1); 
int caseValue=0;
unsigned int eeprom_write_last=0;
int slot=0; 
String step_list[MAX_STEPS] = {"Define     Problem","Research","Brainstorm","Select",
              "Construct","Test","Communicate","Redesign","","","",""};

void setup() 
{

  //SPI.begin();
  Serial.begin(9600); 
  GLCD.Init(); // start the GLCD code
  GLCD.SelectFont(Arial_14);
  GLCD.CursorTo(0,0); 
  attachInterrupt(button_1.interrupt_pin, rise1_funct, RISING);
  attachInterrupt(button_2.interrupt_pin, rise2_funct, RISING); 
  load_screen(); 
}

void loop() 
{ 
  
  if (button_2.pressed) { 
    button_1.pressed = false;    // reset both buttons
    button_2.pressed=false;
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
       // myPDC.createArray(time_1.sectionTime);    // these two cause memory problems
        //myPDC.sendArray();
        break;
      case 2:
        Serial.println("draw graph");
        draw();
        break;
      case 3:
        break;
      }
  } 
  
    int sensorValue = analogRead(sensorPin);//read the potentiometer
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
    if(button_1.pressed) {
      GLCD.GotoXY(87,50);
      GLCD.print("Pause");
    } else {
      GLCD.GotoXY(87,50);
      GLCD.print("     Start     ");
    }
    logTime_1();   
}



//////// FUNCTIONS AND STUFF

// This function displays a simple menu on the screen, which allows the user to change memory slot,
// send times to the docking station, graph the times collected so far, or go back to the timer.
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
    int sensorValue = analogRead(sensorPin);
    int menuValue = caseSelect(sensorValue, 4);
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
    
    if(button_2.pressed) {
      button_2.pressed = false;
      return 3;
    }
    
    if(button_1.pressed) {
      button_1.pressed = false;
      return menuValue;
    }
  } 
}


//maps the analog read output to numbers 0-7
int caseSelect(int sensorValue, int upper)
{
  return(map(sensorValue, 0, 1023, 0, upper-1)); 
}

// save_steps, load_steps used to be here

// check whether the timer should be running or paused, based on the state of button 1
// write the appropriate times to the sectionTime array
void logTime_1()
{ 
  if (button_1.pressed==true){                          // pressed means the timer is running
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


void rise1_funct(){//the interrupt service routine. Calls the rise function in the button1 class. Handles the pause button
  noInterrupts();
  button_1.rise();  
  interrupts();
}

void rise2_funct(){//the interrupt service routine. Calls the rise function in the button2 class. Handles the graph button
  noInterrupts();
  button_2.rise();
  interrupts();
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
    while(button_1.pressed==false){
      delay(25); //waste time, to keep the ardino from using too much power
    } 
   button_1.pressed = false;
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
    GLCD.GotoXY(85,50);
    GLCD.print("Select");
    GLCD.GotoXY(0,50);
    GLCD.print("Clear");
    while (1){
	
	slot=caseSelect(analogRead(sensorPin),9);
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
        if (button_1.pressed) {
          button_1.pressed=false;
          load_times(); 
          break; 
        }
        
        // If they choose clear, call the clear_slot function and continue with the loop
        if (button_2.pressed) {
          button_2.pressed=false;
          clear_slot(); 
          button_1.pressed = false;
          button_2.pressed = false;
          GLCD.ClearScreen();
          old_cursor = -1;
        } 
     }  
}
  
// Ask user to confirm, then  
// overwrite the selected slot of the EEPROM with zeroes.   
void clear_slot()
{ 
  GLCD.ClearScreen();
  GLCD.CursorTo(0,0);
  GLCD.print("Clear    Memory    Slot    "); GLCD.print(slot+1); GLCD.print("?");
  while(1) {  
    GLCD.CursorTo(1,0);
    GLCD.GotoXY(85,50);
    GLCD.print("No");
    GLCD.GotoXY(0,50);
    GLCD.print("Yes");

    if (button_1.pressed) {
      break;
    }
    
    if (button_2.pressed) {
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
