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
    //    test_init = 13;
//    Serial.println("initializing, and got the right one");
    for(int i=0;i<6;i++) {      // initialize transmissionArray to all zeros
        for(int j=0;j<MAXPAIRS*2+3;j++) {
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
// !!! adjust for new arrray size !!!

void PDCsend::printTransmission() {
    long the_code;
    for(int i=0;i<6;i++) {
        for(int j=0;j<number_of_times*2+3;j++) {
            the_code = transmissionArray[i][j];
            if(the_code < 9) {
                Serial.print(the_code, HEX);
            } else {
                Serial.print(convertCodeToKey(the_code));
            }
            Serial.print("   ");
        }
        Serial.println();
    }
}

// takes in an array of time values in order by category, and transfers them to transmissionArray
// In the array, each column is a distinct category-time pair. The first row contains the
// category ID, second column contains the number of digits in the time value (ex .4 for 1000)
// columns 3-7 contain the digits that make up the time value. (ones go in column 3, tens go in
// column 4, etc.)

void PDCsend::createArray(long product_id, int trans_id, unsigned long time_array[]) {
    Serial.println("create array called");
    int column_index;
    
    // first write the triplet of product id, transmission id, checksum
    Serial.print("calling writecolumn with product_id: "); Serial.println(product_id);
    writeColumn(0, product_id);
    writeColumn(1, trans_id);
    writeColumn(2, findCheckSum(time_array));
    
    // this is the main loop for writing times into the array
    for (int i=0; i<number_of_times; i++) {
        //        int theTime = time_array[i];
        column_index = (i*2)+3;
        writeColumn(column_index, i+1);      // first of the pair will be the category number
        writeColumn(++column_index, time_array[i]);
        
    }
}

// this functions sends the IR codes stored in transmissionArray to the docking station
void PDCsend::sendArray() {
    int length;
    int column_index;
    
    // first send the product ID, transmission ID, and checksum separated by commas and
    // followed by a semicolon
    
    for(int i=0; i<3; i++) {
        sendColumn(i);
        if(i == 2) {            // send a semicolon or a comma after each number
            irsend.sendSony(irKeyCodes[11], 32);
            delay(50);
        } else {
            irsend.sendSony(irKeyCodes[10], 32);
            delay(50);
        }
    }
    
    // these nested for loops send the categories and times from the rest of transmissionArray
    
    for (int i=0; i<number_of_times; i++) {
        column_index = (i*2)+3;
        sendColumn(column_index);
        irsend.sendSony(irKeyCodes[10], 32);        // comma
        delay(50);
        sendColumn(++column_index);
        irsend.sendSony(irKeyCodes[11], 32);        // semicolon
        delay(50);
    }
    irsend.sendSony(irKeyCodes[12], 32);            // colon means end of transmission
}

// sendCharArray is meant to pass along a character array sent from the computer to
// the PDC

void PDCsend::sendCharArray(char char_arr[], int length) {
    long code;
    for(int i=0;i<length;i++) {
        code = convertCharToCode(char_arr[i]);
        Serial.println(code, HEX);
        irsend.sendSony(code, 32);
        delay(50);
    }
}

// sendSyncCode sends the product_id followed by a colon. Devices will generally
// keep sending this as a test until they get a reply from the DTD

void PDCsend::sendSyncCode(long product_id) {
    int ID_components[NUM_COMPS] = {0,0,0,0,0};
    int length = checkIntLength(product_id);
    breakItDown(product_id, ID_components);
    
    for(int i=length-1;i>=0;i--) {
        irsend.sendSony(irKeyCodes[ID_components[i]], 32);
        Serial.print(ID_components[i]);
        delay(50);
    }
    Serial.println();
    irsend.sendSony(irKeyCodes[12], 32);
}


// writeColumn takes a column index and an integer and writes each digit of the integer
// into a different row of te column

void PDCsend::writeColumn(int index, long data) {
    Serial.print("index "); Serial.print(index); Serial.print(": ");
    Serial.print(data); Serial.print(", ");
    int time_components[NUM_COMPS];
    int length;
    breakItDown(data, time_components);
    length = checkLength(time_components);
    transmissionArray[0][index] = length;
//    long reconstruct = 0;
    
    
    for(int i=1;i<=length;i++) {
        transmissionArray[i][index]=irKeyCodes[time_components[i-1]];
//        Serial.print("index"); Serial.print(index); Serial.print(": ");
//        Serial.println(transmissionArray[i][index], HEX);
    }
//    for(int i=0;i<length;i++) {
//        reconstruct += convertCodeToKey(transmissionArray[i][index]) * pow(10, i);
//    }
//    Serial.println(reconstruct); reconstruct = 0;
}

// sendColumn takes a column index as an argument and send the numbers in that column
// using the sendSony IR protocol

void PDCsend::sendColumn(int index) {
    int length = transmissionArray[0][index];
//    Serial.print("send length: "); Serial.println(length);
    for(int i=length;i>=1;i--) {
//        Serial.println(transmissionArray[j][index], HEX);
        irsend.sendSony(transmissionArray[i][index], 32);
        delay(50);
    }
}


// findCheckSum checks the time_array to determine how many number will be sent, not
// including the first triplet

// this will have to be changed once the first number of each pair is included in
// the array
int PDCsend::findCheckSum(unsigned long time_array[]) {
    int checksum = 0;
    for(int i=0;i<number_of_times;i++) {
        checksum++;
        checksum += checkIntLength(time_array[i]);
    }
    return checksum;
}

//breakItDown: assign each digit of a number to a spot in the time_components array
// thousands goes in time_components[3], hundreds goes in time_components[2], etc.

void PDCsend::breakItDown(long seconds, int time_components[NUM_COMPS]) {
    for(int i=0;i<NUM_COMPS;i++) {
        time_components[i] = ((seconds%(pow(10,i+1)))/pow(10,i));
    }
}

// returns the number of digits in the time value as determined by time_components

int PDCsend::checkLength(int time_components[NUM_COMPS]) {
    for(int i=(NUM_COMPS-1);i>0;i--) {
        if(time_components[i] != 0) {
            return i+1;
        }
    }
    return 1;
}

int PDCsend::checkIntLength(long item) {
    if(item < 10)
        return 1;
    if(item < 100)
        return 2;
    if(item < 1000)
        return 3;
    if(item < 10000)
        return 4;
    return 5;
}

int PDCsend::convertCodeToKey(long code) {
    
    for( int i=0; i < 14; i++)  {
        if( code == irKeyCodes[i])   {
            return i; // found the key so return it
        }
    }
    return -1;
}

long PDCsend::convertCharToCode(char the_char) {
    for(int i=0;i<13;i++) {
        if(the_char == keyIndex[i]) {
            return irKeyCodes[i];
        }
    }
    Serial.println("error no code found");
    return -1;
}

long PDCsend::pow(int base, int power) {
    long ans = 1;
    for(int i=0;i<power;i++) {
        ans *= base;
    }
    return ans;
}

