



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



#define MAXPAIRS 20
 const long irKeyCodes [11] = { 
        0x18E738A7, //numbers, this is 0; index 0-9
        0x18E748A7,   // 1  
        0x18E768A7,    // 2
        0x18E778A7,       // 3
        0x18E788A7,     // 4
        0x18E798A7,      // 5
        0x18E718B7,       // 6
        0x18E728B7,    // 7
        0x18E738B7,    // 8
        0x18E758B7,    // 9
        0x18E7C8E7};         // end of transmission

class PDCsend {
  public:
  PDCsend();
    PDCsend(int inPin);
    void createArray(unsigned long time_array[]);
    void createArray(unsigned long time_array[],unsigned int cat_array[]);
    void createArray(long cat_time_array[][MAXPAIRS]);
    void sendArray();//int);
    void printTransmission();

    
  private:
    
    static const int NUM_COMPS = 5;
    long transmissionArray[7][MAXPAIRS];
    int checkLength(int[]);
    void breakItDown(long,int[]);
    int convertCodeToKey(long);
    int number_of_times;              // always intializes to 8 in version 1
   IRsend irsend;
};

#endif
