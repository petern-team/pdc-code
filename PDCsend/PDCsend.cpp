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
PDCsend::PDCsend(unsigned int product_id) {
    my_id = product_id;
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
//    long the_code;
    for(int i=0;i<6;i++) {
        for(int j=0;j<num_pairs*2+3;j++) {
//            the_code = transmissionArray[i][j];
//            if(the_code <= 9) {
                Serial.print(transmissionArray[i][j]);
//            } else {
//                Serial.print(convertCodeToKey(the_code));
//            }
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

void PDCsend::createArray(int outgoing_id, unsigned int time_array[]) {
    Serial.println("create array called");
    num_pairs = 8;
    trans_id = outgoing_id;
    int column_index;
    
    // first write the triplet of product id, transmission id, checksum
    Serial.print("calling writecolumn with product_id: "); Serial.println(my_id);
    writeColumn(0, my_id);
    writeColumn(1, trans_id);
    writeColumn(2, findCheckSum(time_array, 8)+8);
    
    // this is the main loop for writing times into the array
    for (int i=0; i<num_pairs; i++) {
        Serial.print("storing time: "); Serial.println(time_array[i]);
        //        int theTime = time_array[i];
        column_index = (i*2)+3;
        writeColumn(column_index, i+1);      // first of the pair will be the category number
        writeColumn(++column_index, time_array[i]);
        
    }
//    num_pairs = number_of_times;
}



// send a complete array or part of a longer array. if trans_id isn't == 0
// send the product_id, trans_id, etc.

void PDCsend::createPartialArray(int outgoing_id, unsigned int data_time_arr[][MAXPAIRS], int length) {
    trans_id = outgoing_id;
    memset(transmissionArray, 0, sizeof(transmissionArray));
    Serial.print("create array called, length: "); Serial.println(length);
    int column_index;
    int checksum;
    num_pairs = length;
    
    // first write the triplet of product id, transmission id, checksum
    if(trans_id != 0) {
        Serial.print("calling writecolumn with product_id: "); Serial.println(my_id);
        writeColumn(0, my_id);
        writeColumn(1, trans_id);
        checksum = findCheckSum(data_time_arr[0], length)+findCheckSum(data_time_arr[1], length);
        Serial.print("checksum is "); Serial.println(checksum);
        writeColumn(2, checksum);
        column_index = 3;
    } else {
        column_index = 0;
    }
    
    // this is the main loop for writing times into the array
    for (int i=0; i<num_pairs; i++) {
        //        int theTime = time_array[i];
        writeColumn(column_index++, data_time_arr[0][i]);
        writeColumn(column_index++, data_time_arr[1][i]);
    }
}

// this functions sends the IR codes stored in transmissionArray to the docking station
// 8/5/13 added the last qualifier(?). let's see how this works out
void PDCsend::sendArray(bool last) {
    int length;
    int column_index=0;
    
    // first send the product ID, transmission ID, and checksum separated by commas and
    // followed by a semicolon
    
    if(trans_id != 0) {
        for(int i=0; i<3; i++) {
            sendColumn(i);
            if(i == 2) {            // send a semicolon or a comma after each number
                sendAndDelay(';');
            } else {
                sendAndDelay(',');
            }
        }
        column_index = 3;
    }
    // these nested for loops send the data pairs from the rest of transmissionArray
    
    for (int i=0; i<num_pairs; i++) {
        sendColumn(column_index++);
        sendAndDelay(',');
        sendColumn(column_index++);
        sendAndDelay(';');
    }
    if(last) 
        irsend.sendSony(':', 32);            // colon means end of transmission
}


// sendCharArray is meant to pass along a character array sent from the computer to
// the PDC

// so far has NOT been tested

void PDCsend::sendCharArray(char char_arr[], int length) {
    long code;
    for(int i=0;i<length;i++) {
        code = char_arr[i];//convertCharToCode(char_arr[i]);
//        Serial.println(code);
        sendAndDelay(code);
    }
}

// sendSyncCode sends the product_id followed by a colon. Devices will generally
// keep sending this as a test until they get a reply from the DTD

void PDCsend::sendSyncCode() {
    int ID_components[NUM_COMPS] = {0,0,0,0,0};
    breakItDown(my_id, ID_components);
    
    for(int i=4;i>=0;i--) {
        sendAndDelay(irKeyCodes[ID_components[i]]);
        Serial.print(ID_components[i]);
    }
    Serial.println();
//    Serial.println(irKeyCodes[12], HEX);
    sendAndDelay(irKeyCodes[12]);
}

// sendConfirm transforms a transmission id into a confirmation code and sends it to
// the DTD

void PDCsend::sendCommand(int trans_id, bool confirm) {
    int product_id_arr[NUM_COMPS];
    int trans_id_arr[3];
    if(confirm)
        trans_id = trans_id%100 + 700;
    
    breakItDown(my_id, product_id_arr);
    for(int i=4;i>=0;i--) {
        sendAndDelay(irKeyCodes[product_id_arr[i]]);   
    }
    sendAndDelay(irKeyCodes[10]);
    
    breakItDown(trans_id, trans_id_arr);
    for(int i=2;i>=0;i--) {
        sendAndDelay(irKeyCodes[trans_id_arr[i]]);
    }
    sendAndDelay(irKeyCodes[12]);
    
}


// writeColumn takes a column index and an integer and writes each digit of the integer
// into a different row of te column

void PDCsend::writeColumn(int index, unsigned int data) {
//    Serial.print("index "); Serial.print(index); Serial.print(": ");
//    Serial.print(data); Serial.println(); // Serial.print(", ");
    int time_components[NUM_COMPS];
    int length;
    breakItDown(data, time_components);
    length = checkLength(time_components);
    transmissionArray[0][index] = length+'0';
//    long reconstruct = 0;
//    Serial.print("length: "); Serial.println(transmissionArray[0][index]);
    
    for(int i=1;i<=length;i++) {
        transmissionArray[i][index]=time_components[i-1]+'0';
//        Serial.print("index"); Serial.print(index); Serial.print(": ");
//        Serial.println(transmissionArray[i][index]);
    }
    
//    for(int i=0;i<length;i++) {
//        reconstruct += convertCodeToKey(transmissionArray[i][index]) * pow(10, i);
//    }
//    Serial.println(reconstruct); reconstruct = 0;
}

// sendColumn takes a column index as an argument and send the numbers in that column
// using the sendSony IR protocol

void PDCsend::sendColumn(int index) {
    int length = transmissionArray[0][index]-'0';
//    Serial.print("send length: "); Serial.println(length);
    for(int i=length;i>=1;i--) {
//        Serial.println(transmissionArray[i][index]);
        sendAndDelay(transmissionArray[i][index]);
    }
}

// finally replace all those annoying irsend lines
void PDCsend::sendAndDelay(char code) {
    unsigned long to_send = code;
    irsend.sendSony(to_send, 32);
    delay(25);
}

// findCheckSum checks the time_array to determine how many number will be sent, not
// including the first triplet

// this will have to be changed once the first number of each pair is included in
// the array
int PDCsend::findCheckSum(unsigned int data_array[], int length) {
    int checksum = 0;
    for(int i=0;i<length;i++) {
//        checksum++;
        checksum += checkIntLength(data_array[i]);
    }
//    Serial.print("checksum: "); Serial.println(checksum);
    return checksum;
}

//breakItDown: assign each digit of a number to a spot in the time_components array
// thousands goes in time_components[3], hundreds goes in time_components[2], etc.

void PDCsend::breakItDown(long data, int data_components[NUM_COMPS]) {
    for(int i=0;i<NUM_COMPS;i++) {
        data_components[i] = ((data%(pow(10,i+1)))/pow(10,i));
//        Serial.print(data_components[i]);
    }
//    Serial.println();
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

long PDCsend::pow(int base, int power) {
    long ans = 1;
    for(int i=0;i<power;i++) {
        ans *= base;
    }
    return ans;
}