#ifndef PDCreceive_h
#define PDCreceive_h

#include "Arduino.h"
#include <IRremote.h>

#ifndef IRKEYCODES
#define IRKEYCODES

const char irKeyCodes[] = {'0','1','2','3','4','5','6','7','8','9',',',';',':'};
//const char keyIndex[13] = {'0','1','2','3','4','5','6','7','8','9',',',';',':'};
const int MAXPAIRS = 50;
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
    bool parseTransmission(unsigned int write_array[][10]);
    
    static const int RECEIVEPIN = 9;
    bool IR_busy;
    bool transmission_complete;
    bool PDC_sync;
//    bool command_query;
//    bool command_write;
    int transmission_id;
    
private:
    static const int maxNumberOfCodes = 100;
    static const int NUMCODES = 14;     //  number of codes in the IR "alphabet"
    unsigned int my_id;
    int length;
    char transmissionArray[100];
    int index;
    //    decode_results results;         // IR data goes here
    
    int checkCharSum(int, bool*);
    bool checkTransmission();
    bool syncCodeRecvd();
    int convertCodeToKey(long);
    void storeData(char key);
    long pow(int, int);
};

#endif