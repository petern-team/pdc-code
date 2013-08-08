/*
 RFsend.h - Library for compiling and sending an array of time values
 from the physical design compass
 
 version 2: include createArray functions to deal with arrays that
 take the order of categories into account
 */

#ifndef RFsend_h
#define RFsend_h

#include "Arduino.h"
#include <VirtualWire.h>


#ifndef IRKEYCODES
#define IRKEYCODES

const char irKeyCodes[] = {'0','1','2','3','4','5','6','7','8','9',',',';',':'};
const int MAXPAIRS = 30;

#endif

class RFsend {
public:
    RFsend();
    RFsend(unsigned int);
    
    // there's a bug in DTDv04 so this has to be defined twice
    static const int MAXPAIRS = 30;
    
    // all createArray functions take in a list of times and put them in a 2D array to be sent
    // to the docking station
    void createArray(int, unsigned int[][MAXPAIRS], int);
//    void createRFArray(int, unsigned int[][MAXPAIRS], int);
    //    void createArray(long cat_time_array[][MAXPAIRS]);
    
    // send all of the times to the docking station using IR codes
//    void sendArray(bool last = true);
    void sendCondensedArray(bool last = true);
    void sendCharArray(char char_arr[], int length);
    void sendSyncCode();
    void sendCommand(int, bool confirm = false);
    
    
    // debugging function used to print transmissionArray to serial on the PDC
    void printTransmission();
    int test_init;
    unsigned int my_id;
    int trans_id;
    
    
private:
    static const int NUM_COMPS = 5;    // maximum number of single-digit time components (1-99999)
    
    // choose one of these next two, depending on if you're using IR or RF
    char transmissionArray[6][MAXPAIRS*2+3]; //start with a maximum size of 5 digits per number
    //    char RFtransmissionArray[14+6*2*MAXPAIRS];
    
    
    int num_pairs;
    // the number of category-time pairs that can be expected in a transmission
    int number_of_times;              // always intializes to 8 in version 1
    //    IRsend irsend;
    // adjust MAXPAIRS to account for header and storing by column
    // the first row will consist of the length of each column
    
    // the sendArray function will know to add a comma or semicolon after each column of digits
    
    // check length takes in the array of time components and returns the number of digits
    // in the time value
    int checkLength(int[]);
    int checkIntLength(long);
    
    void writeColumn(int index, unsigned int data);
    void addColumn(int index, int*, char[]);
//    void sendColumn(int index);
    void sendCondensedColumn(char[], int*);
    void sendRFchar(char code);
    
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