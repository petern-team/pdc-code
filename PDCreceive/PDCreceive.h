#ifndef PDCreceive_h
#define PDCreceive_h

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
    0x18E718C7,    // (comma)
    0x18E728C7,    // (semicolon)
    0x18E738C7,    // (colon)
    0x18E7C8E7};   // end of transmission

class PDCreceive {
public:
    PDCreceive();
//    PDCreceive(int);
    
    void checkIR(IRrecv, decode_results);
    void resetVariables();
    
    static const int MAXPAIRS = 20;
    static const int RECEIVEPIN = 9;
    boolean PDC_in_transmission;
    
private:
    static const int maxNumberOfCodes = 100;
    static const int NUMCODES = 14;     //  number of codes in the IR "alphabet"
    long key;
    int length;
    byte val;
    char transmissionArray[100];
    int index;
    //    decode_results results;         // IR data goes here
    
    int convertCodeToKey(long);
    void storeData(int key);
    void printTransmission();
};

#endif