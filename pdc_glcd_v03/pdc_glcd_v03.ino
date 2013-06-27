/*
 Dominic Guri
 Andrew Whitehead
 Marc Bucchieri
 5/20/13
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


int NUM_STEPS=8;// Number of steps in the design process
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
  
  Serial.begin(9600); 
  GLCD.Init(); // start the GLCD code
  GLCD.SelectFont(Arial_14);
  GLCD.CursorTo(0,0); 
  Serial.println("about to print to screen");
  GLCD.print("TESTING");
  Serial.println("done");
  attachInterrupt(button_1.interrupt_pin, rise1_funct, RISING);
  attachInterrupt(button_2.interrupt_pin, rise2_funct, RISING); 
  load_screen(); 
}

void loop() 
{ 
  
  if (button_2.pressed==true){ 
    button_1.pressed = false;    // pause the timer
    GLCD.ClearScreen();
    GLCD.CursorTo(2,2);
    GLCD.print("Sending     Times");
    myPDC.createArray(time_1.sectionTime);      // myPDC will put all the times and categores in
    myPDC.sendArray();                          // a 2D array and send it to the docking station
    button_2.pressed=false;
    GLCD.ClearScreen();
  } 
    


    int sensorValue = analogRead(sensorPin);//read the potentiometer
    caseValue = caseSelect(sensorValue);    // the current category gets whatever the dial is pointing to
    String section=section_ID(caseValue);
    
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
    GLCD.print("Send    Data");
    
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


//maps the analog read output to numbers 0-7
int caseSelect(int sensorValue)
{
  return(map(sensorValue, 0, 1000, 0, NUM_STEPS-1)); 
}


// return the section ID string based on the position of the dial
String section_ID(int caseValue) // selects the Design Compass step from the 0-7 indexed value. 
{
  return step_list[caseValue]; 
}

//save the step_list array, which contains all of the Design Compass steps
// not used in prototype version 1
void save_steps() 
{
  int cursor_loc=300;
  EEPROM.write(cursor_loc, NUM_STEPS); 
  for (int i=0; i<NUM_STEPS; i++)
  {
    int size_count=0;
    for (int j=0; j<sizeof(step_list[i])-1; j++)
    {
      EEPROM.write(cursor_loc+i, step_list[i][j]);//write the elements of each string to EEPROM memory
      size_count++; 
    }
    cursor_loc+=size_count; //increment the cursor by the size of the written strings
    EEPROM.write(cursor_loc, '~'); //write delimiter
    cursor_loc++; //increment cursor again, this time by size of delimeter (makes the algorithm simpler to understand)
  }
}

// Apparently this function should be used to load previously saved steps from memory.
// In iteration 1 this function is not used because the PDC always uses the same 8 steps
void load_steps()
{
 int cursor_loc=300;
 NUM_STEPS=EEPROM.read(cursor_loc);
 cursor_loc++;
 byte found=0; 
 byte count=0; 
 while(found<NUM_STEPS)
 {
    if(EEPROM.read(cursor_loc)=='~')
    {
      found++;
      count=0; 
    } else {  
      step_list[found][count]=EEPROM.read(cursor_loc); 
      count++;
    }
    cursor_loc++; 
 }
}

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
  button_1.rise(); 
}

void rise2_funct(){//the interrupt service routine. Calls the rise function in the button2 class. Handles the graph button
  button_2.rise();
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
  int hrs=time_1.sectionTime[caseValue]/3600;
  int mins=(time_1.sectionTime[caseValue]%3600)/60;
  int secs=time_1.sectionTime[caseValue]%60;
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
 /*   while(button_2.pressed==true){
      delay(25); //waste time, to keep the ardino from using too much power
    } */
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
  int sensor_val=0;
  int old_cursor=0; 
    GLCD.GotoXY(85,50);
    GLCD.print("Select");
    GLCD.GotoXY(0,50);
    GLCD.print("Clear");
    while (1){
	
	slot=map(analogRead(sensorPin),0,1023,0, 8);
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
