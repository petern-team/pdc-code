E/*
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
 
 SPI code was written using http://www.gammon.com.au/forum/?id=10892&reply=1#reply1 
 as a reference
 */

#include <glcd.h>
#include <allFonts.h>
#include "timer.h" //include the timer struct
#include <EEPROM.h>

const int NUM_STEPS=8;// Number of steps in the design process
const int MAX_STEPS=12; 
timer time_1; // make a new instance of the timer struct
int caseValue=0;
unsigned int eeprom_write_last=0;
int slot=0; 
String step_list[MAX_STEPS] = {"Define     Problem","Research","Brainstorm","Select",
              "Construct","Test","Communicate","Redesign","","","",""};
int old_case; 
//boolean comp_sync;
volatile boolean new_info;
volatile boolean button_1pressed;
volatile boolean button_2pressed;
volatile int sensorValue;
volatile byte incoming_cat;
volatile int incoming_time;
              
//SPI library and variables ----------------------------------------------------------

#include <SPI.h>

int SPI_in[100];
boolean sending_times;
boolean loading_times;
boolean display_sending;
volatile byte pos;
int send_pos;
//volatile byte recv_h_pos;
//volatile boolean recv_v_pos;
volatile boolean SPI_complete;
boolean load_times_complete;


// SPI interrupt routine
// information will be received in triplets of button_1pressed, button_2pressed,
// and sensorValue. At the end of each triplet will be a sentinel (-1)
ISR (SPI_STC_vect)
{
  new_info = true;
  char input = SPDR;
  
  // if the teensy is about to send times, ignore the usual protocol and send teensy
  // the times from the timer
  if(sending_times) {
    SPDR = input + time_1.sectionTime[send_pos];
    send_pos++;
    if(send_pos == 8) {
      send_pos = 0; 
      sending_times = false;
      display_sending = true;
    }
//  } else if(loading_times) {
////    transmission will be category, most significant 8 bits of time, least significant 8
//// bits of time, -1, times as many categories as there are (start as 8)
    
  } else {
  
    // add to buffer if room
    if (pos < sizeof(SPI_in)) {
      SPI_in[pos] = input+0;
      
      // whenever a sentinel of -1 is reached, transmission complete
      if (SPI_in[pos] == -1)
        SPI_complete = true;
        
      pos++;
    }  else {
      Serial.println("Error: buffer full");
    }
  }
}

//--------------------------------------------------------------------------------

void setup() {
//  new_info = false;
  // all of the following is SPI stuff-------------------------------------------------
  // attach SPI interrupts and set "slave out" as an output
  SPI.attachInterrupt();
  pinMode(MISO, OUTPUT);
  
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);
  
  pos=0;
  send_pos=0;
  sending_times = false;
//  recv_h_pos = 0;
//  recv_v_pos = 0;
//  display_sending = false;
  SPI_complete = false;
//  comp_sync = false;
  
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

// if the teensy is busy sending times, display "sending times" for 3 seconds
//  if(display_sending) {
//    GLCD.ClearScreen();
//    GLCD.CursorTo(2,2);
//    GLCD.print("Sending     Times");
//    delay(3000);
//    GLCD.ClearScreen();
//    display_sending = false;
//  }

//if(new_info) {
//  Serial.println("new info");
//  new_info = false;
//}
  

// if button 2 was pressed from the timer, display the menu and interpret results
  if (button_2pressed) { 
    button_1pressed = false;    // reset both buttons
    button_2pressed = false;
    
    // display menu returns whichever option was selected
    int menuSelect = displayMenu();
//    Serial.print("menu returned: "); Serial.println(menuSelect);
    GLCD.ClearScreen();
    
    switch (menuSelect) {
      case 0:
        Serial.println("load screen");
        load_screen();
        break;
      case 1:
        Serial.println("Computer sync menu");
        runSyncScreen();
//        sending_times = true;
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
  String section = step_list[caseValue];
  
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

/******************************************************************************************************
                                       Display Screen Functions
*******************************************************************************************************/


// This function is called either during setup or from the menu. It allows 
// the user to choose a memory slot for the times to be stored in and 
// gives the user a chance to clear any of the memory slots

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

// This function displays the menu that users can access by pressing button 2 from the timer.
// Once in the menu, they have the option to change the active memory slot (or clear memory slots),
// send data via teensy to the docking station, or dispaly a graph showing the amount of time
// spent on each category so far

int displayMenu()
{
  String menuList[] = {"Change    Memory    Slot","Sync    PDC","Display    Graph"};
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

// runSyncScreen displays "searching for docking station" until the DtD is found, and then
// displays "synced with computer". at this point the teensy and arduino will handle information
// input and output

void runSyncScreen() {
  unsigned long last_update = millis();
  GLCD.ClearScreen();
  GLCD.CursorTo(0,1);
  GLCD.print("Searching     for");
  GLCD.CursorTo(0,2);
  GLCD.print("Docking    Station...");
  
  while(1) {
    checkSPI();
    
    if(sensorValue == 30) {
      writeScreen("In     Sync     Mode");
      sensorValue = 29;
    }
    if(sensorValue == 31) {
      writeScreen("No    DTD    Found");
      delay(2000);
      return;
    }
    if(sensorValue == 32) {
      writeScreen("Quitting    Sync    Mode");
      delay(2000);
      return;
    }
    if(sensorValue == 41) {
      sending_times = true;
      writeScreen("Sending     Times");
      delay(3000);
      sensorValue = 30;
    }
    if(sensorValue == 51) {
      writeScreen("Loading    New    Times");
      Serial.println("loading times");
      loadSPItimes();
      delay(2000);
      sensorValue = 30;
    }
    if(sensorValue == 91) {
      writeScreen("Error    Please    Resend");
      delay(2000);
      sensorValue = 30;
    }
  }
  
//  GLCD.ClearScreen();
//  GLCD.CursorTo(0,1);
//  GLCD.print("In     Sync     Mode");
//  while(!quit) {
//    checkSPI();
//  }
}



/*********************************************************************************************************
                                        Support Functions
*********************************************************************************************************/

// writeScreen clears the screen and writes the given string to cursor loaction (0,1)
void writeScreen(String incoming) {
  GLCD.ClearScreen();
  GLCD.CursorTo(0,1);
  GLCD.print(incoming);
}

//loadSPItimes checks for time values coming in through SPI and saves them in the section time array
void loadSPItimes() {
  // use a different checkSPI function to continuously load times into the sectionTime array.
  // when category 8 comes through, set load_complete to true which exits from the while loop
  // and return from the function
  boolean load_complete = false;
  boolean new_times = false;
  byte cat_time_pair[3];
  byte the_cat;
  int the_time;
  
  while(!load_complete) {
    new_times = checkSPItimes(cat_time_pair);
    if(new_times) {
      the_cat = cat_time_pair[0];
      Serial.print("new times, Category: "); Serial.println(the_cat);
      the_time = (cat_time_pair[1] << 8) + cat_time_pair[2];
      Serial.print("time: "); /* Serial.print(cat_time_pair[1] << 8); Serial.print(" + ");*/ Serial.println(the_time);
      time_1.sectionTime[the_cat-1] = the_time;
      if(the_cat == NUM_STEPS)
        load_complete = true;
      new_times = false;
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

// check if an SPI transmission has been completed. If it has, change the variables for
// button 1, button 2, and sensor value accordingly

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
    Serial.print("sensor value: "); Serial.println(sensorValue);
  } 
}

// checkSPItimes is similar to checkSPI but is used to load a new array of times onto 
// the current memory slot of the PDC. The function returns true when a set of three bytes
// (category, time int part 1, time int part 2) has been received

boolean checkSPItimes(byte cat_time_pair[]) {
  if(SPI_complete) {
    for(int i=0;i<3;i++) {
      cat_time_pair[i] = SPI_in[i];
//      Serial.print(cat_time_pair[i]); Serial.print(" ");
    }
//    Serial.println();
    for(int i=pos; i>0; i--) {
      SPI_in[--pos] = 0;
    }
    SPI_complete = false;
    return true;
  }
  return false;
}
  
// maps the analog read output to a new number. Sensorvalue must originally be mapped
// between 0 and 23 so it can be sent over SPI as a byte.
int caseSelect(int sensorValue, int upper)
{
  return(map(sensorValue, 0, 23, 0, upper-1)); 
} 

