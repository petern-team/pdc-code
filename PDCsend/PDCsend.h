/*
 PDCsend.h - Library for compiling and sending an array of time values
 from the physical design compass
 
 version 2: include createArray functions to deal with arrays that
 take the order of categories into account
 */

#ifndef PDCsend_h
#define PDCsend_h

#include "Arduino.h"
#include <IRremote.h>


const long irKeyCodes [14] = {
    0x18E738A7,    //numbers, this is 0; index 0-9
    0x18E748A7,    // 1
    0x18E768A7,    // 2
    0x18E778A7,    // 3
    0x18E788A7,    // 4
    0x18E798A7,    // 5
    0x18E718B7,    // 6
    0x18E728B7,    // 7
    0x18E738B7,    // 8
    0x18E758B7,    // 9
    0x18E718C7,    // (comma) - 10
    0x18E728C7,    // (semicolon) - 11
    0x18E738C7,    // (colon) - 12
    0x18E7C8E7};   // end of transmission - probably won't be used anymore

class PDCsend {
public:
    PDCsend();
    PDCsend(int inPin);
    
    static const int MAXPAIRS = 20;
    
    // all createArray functions take in a list of times and put them in a 2D array to be sent
    // to the docking station
    void createArray(long product_id, int trans_id, unsigned long time_array[]);
    void createArray(unsigned long time_array[],unsigned int cat_array[]);
    void createArray(long cat_time_array[][MAXPAIRS]);
    
    // send all of the times to the docking station using IR codes
    void sendArray();
    
    
    // debugging function used to print transmissionArray to serial on the PDC
    void printTransmission();
    int test_init;
    
    
private:
    static const int NUM_COMPS = 5;    // maximum number of single-digit time components (1-99999)
    
    long transmissionArray[6][MAXPAIRS*2+3]; //start with a maximum size of 5 digits per number
    // adjust MAXPAIRS to account for header and storing by column
    // the first row will consist of the length of each column
    
    // the sendArray function will know to add a comma or semicolon after each column of digits
    
    // check length takes in the array of time components and returns the number of digits
    // in the time value
    int checkLength(int[]);
    int checkIntLength(int);
    
    void writeColumn(int index, long data);
    void sendColumn(int index);
    
    // called by create array to find the checksum number
    int findCheckSum(unsigned long time_array[]);
    
    // breakItDown edits the time_components array to contain the digits that make up the time value,
    // in the order [ones, tens, hundreds, thousands, ten-thousands]
    void breakItDown(long,int[]);
    
    // translates an IR code to its index in the irKeyCodes array
    int convertCodeToKey(long);
    long pow(int, int);
    
    // the number of category-time pairs that can be expected in a transmission
    int number_of_times;              // always intializes to 8 in version 1
    IRsend irsend;
    
    
};

#endif