#ifndef PDCreceive_h
#define PDCreceive_h

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

class PDCreceive {
public:
    PDCreceive();
    PDCreceive(long product);
//    PDCreceive(int);
    
    void checkIR(IRrecv, decode_results);
    void printTransmission();
    void resetVariables();
    char getChar(int index);
    
    static const int MAXPAIRS = 20;
    static const int RECEIVEPIN = 9;
    int PRODUCT_ID;
    bool PDC_in_transmission;
    bool transmission_complete;
    bool PDC_sync;
    
private:
    static const int maxNumberOfCodes = 100;
    static const int NUMCODES = 14;     //  number of codes in the IR "alphabet"
    long key;
    int length;
    char transmissionArray[100];
    int index;
    //    decode_results results;         // IR data goes here
    
    bool syncCodeRecvd();
    int convertCodeToKey(long);
    void storeData(int key);
};

#endif