#include "PDCsend_test.h"


//IRsend irsend;



PDCsend::PDCsend() {


    number_of_times = 8;              // always intializes to 8 in version 1
      for(int i=0;i<7;i++) {      // initialize transmissionArray to all zeros
    for(int j=0;j<MAXPAIRS;j++) {
        transmissionArray[i][j]=0;
      }
  }
}

PDCsend::PDCsend(int inPin) {
    
    pinMode(inPin, INPUT);
    number_of_times = 8;              // always intializes to 8 in version 1
      for(int i=0;i<7;i++) {      // initialize transmissionArray to all zeros
    for(int j=0;j<MAXPAIRS;j++) {
        transmissionArray[i][j]=0;
      }
  }
}

void PDCsend::printTransmission() {
 for(int i=0;i<7;i++) {
  for(int j=0;j<8;j++) {
     if(j == 7) {
        Serial.println(transmissionArray[i][j], HEX);
     } else {
        Serial.print(transmissionArray[i][j], HEX);
        Serial.print("   ");
     } 
  }
 } 
}


void PDCsend::createArray(unsigned long time_array[]) {
      Serial.println("create array called");
      
      int time_components[NUM_COMPS];

//     write the number of characters into the last index of the array
       
       breakItDown(number_of_times, time_components);
       int length = checkLength(time_components);
       transmissionArray[0][MAXPAIRS-1]=irKeyCodes[0];        // first pair sent will be 0, # of characters
       transmissionArray[1][MAXPAIRS-1]=length;

  for(int i=0;i<length;i++) {
   transmissionArray[i+2][MAXPAIRS-1] = irKeyCodes[time_components[i]];
  }
  
   for (int i=0; i<number_of_times; i++) {

  
       long theTime = time_array[i];
//       Serial.print("current time value "); Serial.println(theTime);
       breakItDown(theTime, time_components);
       int length = checkLength(time_components);
       
        // store stuff in the next column of the matrix
       transmissionArray[0][i] = irKeyCodes[i+1];        //store the ID of the category
     // Serial.println(transmissionArray[0][i], HEX); 
       transmissionArray[1][i] = length;             // length is the only one that remains a number       


       // this next block of code stores the digits for the next time value descending 
       //in the selected column of transmissionArray, from ones to thousands
        for(int j=0;j<length; j++) {
   //       Serial.print("time component ");
     //     Serial.print(j); Serial.print(" is "); Serial.print(time_components[j]); Serial.print(", ");
          transmissionArray[j+2][i]=irKeyCodes[time_components[j]]; 
       }   
 //       Serial.println("");       
   }    
}

 void PDCsend::sendArray()//int send_index) {
{
   
   Serial.println("about to send");
   int length;
//   int upper_bound = min(8,send_index);

    //    if(send_index == 0) {
              irsend.sendSony(transmissionArray[0][MAXPAIRS-1], 32);
              Serial.print(transmissionArray[0][MAXPAIRS-1], HEX);Serial.print(", ");
              delay(50);
              length = transmissionArray[1][MAXPAIRS-1];
              irsend.sendSony(irKeyCodes[length], 32);
              Serial.print(irKeyCodes[length], HEX);Serial.print(", ");
              delay(50);
              for(int i=0;i<length;i++) {
                irsend.sendSony(transmissionArray[i+2][MAXPAIRS-1], 32);
                Serial.println(transmissionArray[i+2][MAXPAIRS-1], HEX);
                delay(50);
              }
   //     }

      for (int i=0; i<number_of_times; i++) {         //i is the index of the column being sent

         irsend.sendSony(transmissionArray[0][i], 32);
  //       Serial.print(transmissionArray[0][i], HEX);
    //     Serial.print(", ");
         delay(50);
         length = transmissionArray[1][i];
         irsend.sendSony(irKeyCodes[length], 32);
   //      Serial.print(irKeyCodes[length], HEX);Serial.print(", ");
         delay(50);
     
         for (int j=0; j<length; j++) {                //j is the index of the row being sent

             irsend.sendSony(transmissionArray[j+2][i], 32);
             delay(50);
         }
         
       if(i == 7) {
             irsend.sendSony(irKeyCodes[10], 32);            //end of transmission
                    delay(50);
       }
   }


 

 }

//breakItDown: assign each digit of a number to a spot in the time_components array
// thousands goes in time_components[3], hundreds goes in time_components[2], etc.

void PDCsend::breakItDown(long seconds, int time_components[NUM_COMPS]) { 

long seconds_comp;

for(int i=NUM_COMPS-1;i>=0;i--) {
  long comp_factor=1;
  for(int j=0;j<i;j++) {
    comp_factor = comp_factor*10;
  }
  
  seconds_comp = seconds/comp_factor;
  if(seconds_comp != 0)
    time_components[i] = seconds_comp;
  else
    time_components[i] = 0;
  
  seconds -= time_components[i]*comp_factor;  
}   
}

// checkLength(): change the length variable to the number of digits in time_components

int PDCsend::checkLength(int time_components[NUM_COMPS]) 
{
 for(int i=(NUM_COMPS-1);i>0;i--) {
    if(time_components[i] != 0) {
       return i+1; 
    }
 }
 return 1;
}

int PDCsend::convertCodeToKey(long code)
{     

  for( int i=0; i < 11; i++)  {
    if( code == irKeyCodes[i])   {
      return i; // found the key so return it
    }
  }
  return -1;
}


