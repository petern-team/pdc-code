//
//  PDCsend.cpp
//  
//
//  Created by Marc Bucchieri on 6/28/13.
//
//

#include "PDCsend.h"
#include <IRremote.h>


// initialize transmissionArray to all zeros
PDCsend::PDCsend() {
    number_of_times = 8;              // always intializes to 8 in version 1
    for(int i=0;i<7;i++) {      // initialize transmissionArray to all zeros
        for(int j=0;j<MAXPAIRS;j++) {
            transmissionArray[i][j]=0;
        }
    }
}

// initialize transmissionArray to all zeros and set the specified pin number as an input pin
PDCsend::PDCsend(int inPin) {
    
    pinMode(inPin, INPUT);
    number_of_times = 8;              // always intializes to 8 in version 1
    for(int i=0;i<7;i++) {      // initialize transmissionArray to all zeros
        for(int j=0;j<MAXPAIRS;j++) {
            transmissionArray[i][j]=0;
        }
    }
}

// print the transmissionArray line by line to serial (used for debugging purposes)
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

// takes in an array of time values in order by category, and transfers them to transmissionArray
// In the array, each column is a distinct category-time pair. The first row contains the
// category ID, second column contains the number of digits in the time value (ex .4 for 1000)
// columns 3-7 contain the digits that make up the time value. (ones go in column 3, tens go in
// column 4, etc.)

void PDCsend::createArray(unsigned long time_array[]) {
    Serial.println("create array called");
    
    int time_components[NUM_COMPS];
    
    // write the number of caegory-time pairs into the last index of the array, paired with category 0
    breakItDown(number_of_times, time_components);
    int length = checkLength(time_components);           // in version 1, always returns 1 because there are
    // always 8 categories
    transmissionArray[0][MAXPAIRS-1]=irKeyCodes[0];        // first pair sent will be 0, # of characters
    transmissionArray[1][MAXPAIRS-1]=length;
    
    for(int i=0;i<length;i++) {
        transmissionArray[i+2][MAXPAIRS-1] = irKeyCodes[time_components[i]];
    }
    
    
    // this is the main loop for writing times into the array
    for (int i=0; i<number_of_times; i++) {
        long theTime = time_array[i];
        breakItDown(theTime, time_components);
        int length = checkLength(time_components);
        
        // store stuff in the current column of the matrix
        transmissionArray[0][i] = irKeyCodes[i+1];    //store the ID of the category
        transmissionArray[1][i] = length;             // length is the only one that remains a number
        
        // this next block of code stores the digits for the next time value descending
        //in the selected column of transmissionArray, from ones to thousands
        for(int j=0;j<length; j++) {
            transmissionArray[j+2][i]=irKeyCodes[time_components[j]];
        }
    }
}

// this functions sends the IR codes stored in transmissionArray to the docking station
void PDCsend::sendArray() {
    int length;
    
    // this next block of code sends the pair (0, number of times) so the receiver knows how many to
    // wait for
    
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
    
    // these nested for loops send the categories and times from the rest of transmissionArray
    
    for (int i=0; i<number_of_times; i++) {         //i is the index of the column being sent
        
        irsend.sendSony(transmissionArray[0][i], 32);
        delay(50);
        length = transmissionArray[1][i];
        irsend.sendSony(irKeyCodes[length], 32);
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

// returns the number of digits in the time value as determined by time_components

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

