// This code handles the reception and interpretation of code on the
// data transfer device

#ifndef PDCreceive_h
#define PDCreceive_h

#include "Arduino.h"
#include <IRremote.h>
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

class PDCreceive {
public:
    PDCreceive();
    PDCreceive(int);
    
    void checkIR(IRrecv, decode_results);
    void resetVariables();
    
    static const int MAXPAIRS = 20;
    static const int RECEIVEPIN = 9;
    boolean PDC_in_transmission;
    
    //  private:
    static const int maxNumberOfCodes = 100;
    static const int NUMCODES = 11;     //  number of codes in the IR "alphabet"
    //    long key;
    int length;
    //    byte val;
    long transmissionArray[2][maxNumberOfCodes];
    int h_index;
    int v_index;
    //    decode_results results;         // IR data goes here
    
    int convertCodeToKey(long);
    int translateCodes1(int);
    int translateCodes2(int);
    int translateCodes3(int);
    void printTransmission();
};

#endif