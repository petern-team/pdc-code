//
//  RFsend.cpp
//
//
//  Created by Marc Bucchieri on 6/28/13.
//
//

#include "RFsend.h"
//#include <IRremote.h>


// initialize transmissionArray to all zeros
RFsend::RFsend() {
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
RFsend::RFsend(unsigned int product_id) {
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

void RFsend::printTransmission() {
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




// send a complete array or part of a longer array. if trans_id isn't == 0
// send the product_id, trans_id, etc.

void RFsend::createArray(int outgoing_id, unsigned int data_time_arr[][MAXPAIRS], int length) {
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

// different version of sendArray that condenses the transmissionArray into character arrays of up to 27
// characters and sends those one at a time

void RFsend::sendCondensedArray(bool last) {
//    int length;
    int column_index=0;
    char msg[27];
    int msg_index = 0;
    
    // first send the product ID, transmission ID, and checksum separated by commas and
    // followed by a semicolon
    
    if(trans_id != 0) {
        for(int i=0; i<3; i++) {
            addColumn(i, &msg_index, msg);
            if(i == 2) {            // send a semicolon or a comma after each number
                msg[msg_index++] = ';';
            } else {
                msg[msg_index++] = ',';
            }
        }
        column_index = 3;
    }
    // these nested for loops send the data pairs from the rest of transmissionArray
    
    for (int i=0; i<num_pairs; i++) {
        addColumn(column_index++, &msg_index, msg);
        msg[msg_index++] = ',';
        addColumn(column_index++, &msg_index, msg);
        msg[msg_index++] = ';';
        if(msg_index > 14)
            sendCondensedColumn(msg, &msg_index);
    }
    if(last)
        msg[msg_index++] = ':';            // colon means end of transmission
    sendCondensedColumn(msg, &msg_index);
}


// sendCharArray is meant to pass along a character array sent from the computer to
// the PDC

void RFsend::sendCharArray(char char_arr[], int length) {
    long code;
    char msg[VW_MAX_MESSAGE_LEN];
    int msg_index = 0;
    for(int i=0;i<length;i++) {
        msg[msg_index] = char_arr[i];
        msg_index++;
        if(msg_index == 26)
            sendCondensedColumn(msg, &msg_index);
    }
    sendCondensedColumn(msg, &msg_index);
}

// sendSyncCode sends the product_id followed by a colon. Devices will generally
// keep sending this as a test until they get a reply from the DTD

void RFsend::sendSyncCode() {
    int ID_components[NUM_COMPS] = {0,0,0,0,0};
    breakItDown(my_id, ID_components);
    int arr_size = 6;
    char msg[6];
    
    
    for(int i=0;i<5;i++) {
        msg[i] = ID_components[4-i]+'0';
//        sendRFchar(irKeyCodes[ID_components[i]]);
        //        Serial.print(ID_components[i]);
    }
    msg[5] = ':';
    sendCondensedColumn(msg, &arr_size);
//    Serial.println();
    //    Serial.println(irKeyCodes[12], HEX);
//    sendRFchar(':');
}

// sendConfirm transforms a transmission id into a confirmation code and sends it to
// the DTD

void RFsend::sendCommand(int trans_id, bool confirm) {
    int product_id_arr[NUM_COMPS];
    int trans_id_arr[3];
    if (confirm)
        trans_id = trans_id%100 + 700;
    
    breakItDown(my_id, product_id_arr);
    for(int i=4;i>=0;i--) {
        sendRFchar(irKeyCodes[product_id_arr[i]]);
    }
    sendRFchar(irKeyCodes[10]);
    
    breakItDown(trans_id, trans_id_arr);
    for(int i=2;i>=0;i--) {
        sendRFchar(irKeyCodes[trans_id_arr[i]]);
    }
    sendRFchar(irKeyCodes[12]);
    
}


// writeColumn takes a column index and an integer and writes each digit of the integer
// into a different row of te column

void RFsend::writeColumn(int index, unsigned int data) {
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
}

// add a column from transmission array to the char msg[] array about to be sent over RF

void RFsend::addColumn(int index, int *msg_index, char msg[]) {
    int length = transmissionArray[0][index]-'0';
//    Serial.print("adding to "); Serial.print(*msg_index);
    for(int i=length;i>=1;i--) {
        msg[*msg_index] = transmissionArray[i][index];
        (*msg_index)++;
    }
//    Serial.print("... "); Serial.print(*msg_index); Serial.println();
}

// send up to 27 codes that have been condensed into a string
void RFsend::sendCondensedColumn(char msg[], int *msg_index) {
//    Serial.print("sending: "); Serial.println(msg);
    vw_send((uint8_t *)msg, *msg_index);
    vw_wait_tx();
    (*msg_index) = 0;
}

// RF?
void RFsend::sendRFchar(char code) {
    Serial.print(code);
    char msg[1];
    msg[0] = code;
    vw_send((uint8_t *)msg, 1);
    //    vw_wait_tx(); // Wait until the whole message is gone
}


// findCheckSum checks the time_array to determine how many number will be sent, not
// including the first triplet

// this will have to be changed once the first number of each pair is included in
// the array
int RFsend::findCheckSum(unsigned int data_array[], int length) {
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

void RFsend::breakItDown(long data, int data_components[NUM_COMPS]) {
    for(int i=0;i<NUM_COMPS;i++) {
        data_components[i] = ((data%(pow(10,i+1)))/pow(10,i));
        //        Serial.print(data_components[i]);
    }
    //    Serial.println();
}

// returns the number of digits in the time value as determined by time_components

int RFsend::checkLength(int time_components[NUM_COMPS]) {
    for(int i=(NUM_COMPS-1);i>0;i--) {
        if(time_components[i] != 0) {
            return i+1;
        }
    }
    return 1;
}

int RFsend::checkIntLength(long item) {
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

long RFsend::pow(int base, int power) {
    long ans = 1;
    for(int i=0;i<power;i++) {
        ans *= base;
    }
    return ans;
}