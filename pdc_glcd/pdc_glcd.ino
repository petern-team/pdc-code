/*

 Dominic Guri
 Andrew Whitehead
 7/11/12
 Physical Design Compass Potentiometer Wheel Prototype
 Tufts Center for Engineering Education and Outreach
 GLCD library will be included with the code. In addition,
 it can be downloaded from http://code.google.com/p/glcd-arduino/downloads/detail?name=glcd-v3-20111205.zip
 */
#include <glcd.h>
#include <fonts/allFonts.h>
#include "timer.h" //include the timer struct
#include "button.h" //include the button class
#include <EEPROM.h>
int NUM_STEPS=8;// Number of steps in the design process
const int MAX_STEPS=15; 
int sensorPin = A5; //the pin on the Arduino that the middle (wiper) pin of the potentiometer is connected to
const int READ_MAX=1023; // the maximum value analogRead can have, used to map the range of values from the potentiometer
int  old_case=-1;  //the default case for the DC to begin
timer time_1; // make a new instance of the timer struct
button button_1(0); //make a new instance of the button class
button button_2(1); 
int caseValue=0;
unsigned int eeprom_write_last=0;
int heights[MAX_STEPS]; 
int slot=0; 
String step_list[MAX_STEPS];
void setup() 
{
  Serial.begin(115200); 
  GLCD.Init(); // start the GLCD code
  GLCD.SelectFont(Arial_14);
  GLCD.CursorTo(0,0); 
  attachInterrupt(button_1.interrupt_pin, rise1_funct, RISING);
  attachInterrupt(button_2.interrupt_pin, rise2_funct, RISING); 
  load_screen(); 
  for (int i=0; i<MAX_STEPS; i++){
    heights[i]=0;
  }

}
void loop() 
{ 
  if (button_2.pressed==true){ //call graph function if the button was pressed (managed by interrupt)
    get_heights(); 
    draw();
    return; 
    }
  if (Serial.available())
  {
     Serial.read();
     for(int j=0; j<NUM_STEPS; j++)
         {
            Serial.println(time_1.sectionTime[j]); 
         }
  }
  if (Serial.available())
  {
    NUM_STEPS=Serial.read();
    char buffer[30];
    for(int i=0; i<NUM_STEPS; i++)
    {
      int count=0; 
      while(Serial.peek()!='~')
      {
        buffer[count]=Serial.read();
        count++;
      }
      for(int j=0; i<count; j++)
      {
        step_list[i]+=buffer[j];
      }
    }
  }
  int sensorValue = analogRead(sensorPin);//read the potentiometer
  caseValue = caseSelect(sensorValue);
  String section=section_ID(caseValue);
  if (caseValue!=old_case) //logic used to avoid having to clear the screen every iterartion.
    //allows the arduino to remember what the last section it timed was. 
  {
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
  GLCD.GotoXY(85,50);
  GLCD.print("Slot     "); GLCD.print(slot+1);
  logTime_1 (); //call the log time function;                 
}
int caseSelect(int sensorValue)
{
  return(map(sensorValue, 0, 1023, 0, NUM_STEPS-1)); //maps the analog read output to numbers 0-7
}


//No good way around this yet. 
String section_ID(int caseValue) // selects the Design Compass step from the 0-7 indexed value. 
{
  String str;
  switch (caseValue) {
  case 0:
    str=String("Problem");
    break;
  case 1:
    str=String("Research");
    break;
  case 2:
    str=String("Brainstorm");
    break;
  case 3:
    str=String("Select");
    break;
  case 4:
    str=String("Construct");
    break;
  case 5:
    str=String("Test");
    break;
  case 6:
    str=String("Communicate");
    break;
  case 7:
    str=String("Redesign");
    break;
  } 
  return str; 
}

void save_steps() //save the step_list array, which contains all of the Design Compass steps
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
    }
    else 
    {  
      step_list[found][count]=EEPROM.read(cursor_loc); 
      count++;
    }
    cursor_loc++; 
 }
}
  void logTime_1()
{ 
  if (button_1.pressed==true){
    time_1.time_val = millis()/ 1000; //get the current time
    time_1.storedTime = time_1.sectionTime[caseValue]; // store the time that was previously in that section
    time_1.sectionTime[caseValue] = time_1.storedTime + (time_1.time_val -time_1.lastTime);
    //add the time elapsed in this section to the proper array element. 
    if(time_1.time_val-eeprom_write_last==10){
      save_times();
       eeprom_write_last=millis()/1000; 
       }    
    time_1.lastTime = time_1.time_val; //update the lastTime field with the time that this function started
    if (time_1.lastSection!=caseValue)
      {
     if (time_1.time_val-time_1.last_section_time>5)
         {
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

void save_times(){//The algorithm to save the time values into EEPROM memory. Only works with short ints now. 

/* the function takes the high and low bytes of each stored integer, and then saves them sequentially in EEPROM 
memory*/
    for(int i=0; i<NUM_STEPS; i++){
  //  if (i<NUM_STEPS){
		byte hibyte=highByte(time_1.sectionTime[i]);
		byte lobyte=lowByte(time_1.sectionTime[i]);
		EEPROM.write((2*i)+(slot*(NUM_STEPS)), hibyte);
		EEPROM.write(2*i+(slot*(NUM_STEPS))+1, lobyte);
		//}
	/*else{
		EEPROM.write((2*NUM_STEPS+i-NUM_STEPS+slot*(NUM_STEPS+50)), time_1.steps[i-NUM_STEPS]);
            }*/
    }
  //TO SAVE STEP CHANGES, TREAT AS ONE LARGE ARRAY OF SIZE 2*NUM_STEPS+50, SAVE SEQUENTIALLY. USE IF LOGIC TO SAVE THE FIRST SET OF BYTES, ELSE LOGIC TO SAVE THE CHANGE ARRAY
 }
void load_times(){ //this function takes each of the sequentially written bytes and converts them into an interger via the word function. 
//this is used to load the saved time values. 
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

void load_times(int loc){ //this function takes each of the sequentially written bytes and converts them into an interger via the word function. 
//this is used to load the saved time values. 
  for(int i=0; i<NUM_STEPS;  i++){
    //if (i<NUM_STEPS){
      time_1.sectionTime[i]=word(EEPROM.read(2*i+(loc*(NUM_STEPS))), EEPROM.read(2*i+(loc*(NUM_STEPS)+1)));
   // }
 /* else{
     time_1.steps[2*i-NUM_STEPS]=EEPROM.read(2*NUM_STEPS+i-NUM_STEPS+slot*(NUM_STEPS+50)); 
    }
    */
  }
  
}
void draw(){//this is the algorithm to draw the bar graphs on the screen
  GLCD.SelectFont(System5x7); 
  GLCD.ClearScreen();
  get_heights(); //gets the heights for each column
  GLCD.DrawHLine(0,54, 76); //draws the boundary lines for the graph
  GLCD.DrawVLine(76,0,63);
  int sum=sum_times(); 
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
    while(button_2.pressed==true){
      delay(25); //waste time, to keep the ardino from using too much power
    }
   GLCD.ClearScreen(); 
}

void get_heights (){
  int total_time=sum_times(); 
  for (int i=0; i<NUM_STEPS; i++){
    heights[i]=map(floor(time_1.sectionTime[i]), 0, total_time, 0, 40);
    }
}

void load_screen(){// This is the code to handle the load screen
  int sensor_val=0;
  int old_cursor=0; 
  while (1){
	GLCD.CursorTo(0,0);
	slot=map(analogRead(sensorPin),0,1023,0, 8);
	if (slot!=old_cursor) //logic used to avoid having to clear the screen every iterartion.
        //allows the arduino to remember what the last cursor number it timed was. 
        {       
          GLCD.ClearScreen();
          old_cursor=slot;
        }
        GLCD.print("Slot     "); GLCD.print(slot+1);
        if (button_1.pressed==true)
        {//load an old time value
          load_times(); 
          break; 
        }
        if (button_2.pressed==true){
        clear_slot(); // overwrite an old save slot. 
        load_times(); 
        break; 
        }
     }
     button_1.pressed=false;
     button_2.pressed=false; 
  }
  
void clear_slot (){ //overwrite the selected slot of the EEPROM with zeroes. 
   for(int i=0; i<(NUM_STEPS*2); i++){
       EEPROM.write(i+(slot*(NUM_STEPS)), 0);
  }  
}

int sum_times(){
  int sum=0;
  for (int i=0; i<NUM_STEPS; i++){
    sum+=time_1.sectionTime[i];
  }
   return sum; 
}
