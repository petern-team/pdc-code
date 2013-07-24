/*

This sketch records the IR codes as it receives them and stores them in an array
After the end of each transmission it prints the array to serial
This sketch will only work if there are at least 2 seconds between transmissions

 */

#include <IRremote.h>           // IR remote control library

const int irReceivePin = 8;     // pin connected to IR detector output
const int numberOfKeys = 19;     //  how many keys you want to learn
const int maxNumberOfCodes = 100;

// const long irKeyCodes [11] = { 
//        0x18E738A7, //numbers, this is 0; index 0-9
//        0x18E748A7,   // 1  
//        0x18E768A7,    // 2
//        0x18E778A7,       // 3
//        0x18E788A7,     // 4
//        0x18E798A7,      // 5
//        0x18E718B7,       // 6
//        0x18E728B7,    // 7
//        0x18E738B7,    // 8
//        0x18E758B7,    // 9
//        0x18E7C8E7};         // end of transmission

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
    0x18E7C8E7};

int value;
int timeOfLast = 0;
long transmissionArray[maxNumberOfCodes];
int index = 0;
long key;

IRrecv irrecv(irReceivePin);    // create the IR library
decode_results results;         // IR data goes here


void setup()
{

  Serial.begin(9600);
  pinMode(irReceivePin, INPUT);
  irrecv.enableIRIn();              // Start the IR receiver

 
Serial.println("serial works");

}

void loop() {
 if (millis() - timeOfLast > 1000) {        //if its been 2 seconds since the last received data
   for (int i=0; i<index; i++) {        // print all the codes in transmission array and clear the array
//     Serial.print(i+1);   
     Serial.print(transmissionArray[i], HEX);
     Serial.print(": ");
     Serial.println(convertCodeToKey(transmissionArray[i]));
     transmissionArray[i] = 0;
   }  
   index = 0;
 }

  if (irrecv.decode(&results)) {
    Serial.println("got one");
    // here if data is received
    irrecv.resume();
//    showReceivedData();
    key = results.value;
    storeCodes(key);
  } 
}

//translate the code to a section on the PDC or a number

void storeCodes(long key) {
  transmissionArray[index] = key;
  index++;
  timeOfLast = millis();
}

int convertCodeToKey(long code) {     
  for( int i=0; i < 14; i++)  {
    if( code == irKeyCodes[i])   {
      return i; // found the key so return it
    }
  }
  return -1;
}

