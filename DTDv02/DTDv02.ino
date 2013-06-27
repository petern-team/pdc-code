/*
This sketch decodes IR messages written with category/ time pairs.
Then it prints an opening sequence to the computer, followed by a 
character number (sometimes) and all the number pairs.
 */

#include <IRremote.h>           // IR remote control library
#include "audioCommPin.h"

boolean PDC_in_transmission; // uses true to indicate that LEDs need to be switched

const int RECEIVEPIN = 9;     // pin connected to IR detector output
const int NUMCODES = 11;     //  how many keys you want to learn
const int maxNumberOfCodes = 100;
const String transmission_sequence = "abcdefg";
const int redLED = 3;
const int greenLED = 2;
const long irKeyCodes[NUMCODES] = {
    0x18E738A7, //from here down are all numbers, this is 0
    0x18E748A7,     // 1
    0x18E768A7,    // 2
    0x18E778A7,       // 3
    0x18E788A7,     // 4
    0x18E798A7,      // 5
    0x18E718B7,       // 6
    0x18E728B7,      // 7
    0x18E738B7,      // 8
    0x18E758B7,     // 9
    0x18E7C8E7 // end of transmission  
};

int length;
int val;
long transmissionArray[2][maxNumberOfCodes];
int h_index;
int v_index;

IRsend irsend;
IRrecv irrecv(RECEIVEPIN);    // create the IR library
decode_results results;         // IR data goes here

void setup()
{
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  Serial.begin(9600);

  pinMode(RECEIVEPIN, INPUT);

  irrecv.enableIRIn();              // Start the IR receiver
 
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  
  PDC_in_transmission = false;
  
  // initialize variables and timers for audio-wire communication
  resetStorage();
  initAudioPin();  
  
  // initialize sketch variables
  for(int i=0;i<2;i++) {
    for(int j=0;j<maxNumberOfCodes;j++) {
      transmissionArray[i][j]=0;
    } 
  }
  h_index = 0;
  v_index = 0;
  val = 1;
}



void loop()
{
  checkLEDstate();
   
  // check for communication through the audio wire  
  if(checkTransmission()) {
    parseArray();
    printArray();
    resetStorage();
    resetChars();
  }
 
  long key;
    
  if (irrecv.decode(&results)) {  
    PDC_in_transmission = true;
    Serial.println(results.value, HEX);
    // here if data is received
    irrecv.resume();

//    showReceivedData();
    
    key = convertCodeToKey(results.value);
    switch (val) {
    case 1: 
      val = translateCodes1(key);
      break;
    case 2: 
      val = translateCodes2(key);
      break;  
    case 3:
      val = translateCodes3(key);
      break;
    }
  }
  
//  if(digitalRead(BUTTONPIN)) {
//    resetVariables();
//  } 
}

//translate the code to a section on the PDC or a number

int translateCodes1(long key) {
  
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
  
//   if(key == 0 && h_index >= 9)
//     h_index = 0;

   if (key == 10) {
     printTransmission();
     return 1;
   } else {
     Serial.print("from 1: ");
     Serial.println(key);
     transmissionArray[0][h_index] = key;
     return 2;
   }
     
}

int translateCodes2(long key) {  
   if (key == 10) {
     printTransmission();
     return 1;
   }
  length = key;
  Serial.print("from 2: ");
  Serial.println(key);
  return 3;
}

int translateCodes3(long key) {
  long number = 0;
   if (key == 10) {
     printTransmission();
     return 1;
   }
  
  if (v_index < length) {
    
    // if the length is four multiply by 1000, if its 3 multiply by 100, etc. then do length = length -1
    // this function should never be called when length = 0, length should always be redefined before that point
    number = key;
    for (int i=0; i<v_index; i++) {
      key *= 10;
    }
    transmissionArray[1][h_index] += key;
    v_index++;
    
    if (v_index == length) {
      h_index++;
      v_index = 0;
      return 1;
    } else {
      return 3;
    }   
  } else {
    Serial.println("Error in 3");
    length = 0;
    v_index = 0;
    return 1;
  }
}


//print the array when the end of transmission code is received

void printTransmission() {
  
   Serial.println("print transmission called");
   Serial.print("index = "); Serial.println(h_index);
//   delay(10);
  
  for (int i=0; i<h_index; i++) {
    Keyboard.print(transmissionArray[0][i]);
//    delay(10);
    Keyboard.print(",");
//    delay(10);
    Keyboard.print(transmissionArray[1][i]);
//    delay(10);
    if (i == h_index-1) {
      Keyboard.print(":");
//      delay(10);
    } else {
      Keyboard.print(";");
//      delay(10);
    }
  }
  resetVariables();
}

void resetVariables() {
  Serial.println("RESETTING VARIABLES");
   for(int i=0;i<2;i++) {
     for(int j=0;j<maxNumberOfCodes;j++) {
       transmissionArray[i][j]=0;
     } 
   }
  h_index = 0;                  // this can cause problems sometimes !!!!!!
  PDC_in_transmission = false;
  v_index=0;
  val=1;
}

void checkLEDstate() {
  if(PDC_in_transmission && digitalRead(greenLED)) {
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
  } else if(!PDC_in_transmission && digitalRead(redLED)) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
  }
}

//counts the number of characters in transmissionArray, plus commas and semicolons

int countCharacters(int num_cols) {
  int num_chars=0;
  num_chars=num_cols*4;    // two numbers per column, one comma/semicolon/colon per number
  return num_chars;
}

/*
 * converts a remote protocol code to a logical key code 
 * (or -1 if no digit received)
 */

int convertCodeToKey(long code) {
  for( int i=0; i < NUMCODES; i++) {
    if( code == irKeyCodes[i]) {
      return i; // found the key so return it
    }
  }
  return -1;
}
