#ifndef AUDIO_COMM_PIN_h
#define AUDIO_COMM_PIN_h

#include "Arduino.h"

#define SEARCH2 3   
#define SEARCH3 7
#define SEARCH4 15
#define SEARCH6 63
#define SEARCH8 255
#define SEARCH12 4095
#define SEARCH16 65535

#define FINDGROUP SEARCH3

const byte HEADER = B100011;
const byte TAIL = B111111;
const int ARR_SIZE = 400;
const int BUTTONPIN = 6;

void initAudioPin();
void resetStorage();
void pinChange();
int getTime();
boolean checkComplete();
void resetStorage();
void resetChars();
void parseArray(char char_arr[], int *char_index);
void printArray(char char_arr[], int *char_index);
void findBuffer(int*,byte*,byte);
char getChar(int, byte);
int pow(int,int);



#endif
