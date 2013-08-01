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


#ifndef IRKEYCODES
#define IRKEYCODES
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
    0x18E718C7,    // (comma)
    0x18E728C7,    // (semicolon)
    0x18E738C7,    // (colon)
    0x18E7C8E7};   // end of transmission

const char keyIndex[13] = {'0','1','2','3','4','5','6','7','8','9',',',';',':'};
#endif

class PDCsend {
public:
    PDCsend();
    PDCsend(unsigned int);
    
    static const int MAXPAIRS = 20;
    
    // all createArray functions take in a list of times and put them in a 2D array to be sent
    // to the docking station
    void createArray(int trans_id, unsigned int time_array[]);
//    void createArray(int trans_id, char info_array[]);
//    void createArray(unsigned long time_array[],unsigned int cat_array[]);
    void createDataArray(int trans_id, unsigned int data[], unsigned int time_stamps[], int length);
//    void createArray(long cat_time_array[][MAXPAIRS]);
    
    // send all of the times to the docking station using IR codes
    void sendArray();
    void sendCharArray(char char_arr[], int length);
    void sendSyncCode(long);
    void sendConfirm(long, int);
    
    
    // debugging function used to print transmissionArray to serial on the PDC
    void printTransmission();
    int test_init;
    unsigned int my_id;
    
    
private:
    static const int NUM_COMPS = 5;    // maximum number of single-digit time components (1-99999)
    
    int transmissionArray[6][MAXPAIRS*2+3]; //start with a maximum size of 5 digits per number
    int num_pairs;
    // the number of category-time pairs that can be expected in a transmission
    int number_of_times;              // always intializes to 8 in version 1
    IRsend irsend;
    // adjust MAXPAIRS to account for header and storing by column
    // the first row will consist of the length of each column
    
    // the sendArray function will know to add a comma or semicolon after each column of digits
    
    // check length takes in the array of time components and returns the number of digits
    // in the time value
    int checkLength(int[]);
    int checkIntLength(long);
    
    void writeColumn(int index, int data);
    void sendColumn(int index);
    void sendAndDelay(long code);
    
    // called by create array to find the checksum number
    int findCheckSum(unsigned int data_array[], int length);
    
    // breakItDown edits the time_components array to contain the digits that make up the time value,
    // in the order [ones, tens, hundreds, thousands, ten-thousands]
    void breakItDown(long,int[]);
    
    // translates an IR code to its index in the irKeyCodes array
    int convertCodeToKey(long);
    long convertCharToCode(char);
    long pow(int, int);
    

    
    
};

#endif