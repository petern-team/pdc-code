#include "PDCreceive.h"
#include <IRremote.h>

PDCreceive::PDCreceive() {
  pinMode(RECEIVEPIN, INPUT);
//  irrecv.enableIRIn();              // Start the IR receiver
  
  for(int i=0;i<2;i++) {
    for(int j=0;j<maxNumberOfCodes;j++) {
      transmissionArray[i][j]=0;
    } 
  }
  h_index = 0;
  v_index = 0;
  val = 1;
  PDC_in_transmission = false;
}

PDCreceive::PDCreceive(int receive_pin) {
  pinMode(receive_pin, INPUT);
//  irrecv.enableIRIn();              // Start the IR receiver
  
  for(int i=0;i<2;i++) {
    for(int j=0;j<maxNumberOfCodes;j++) {
      transmissionArray[i][j]=0;
    } 
  }
  h_index = 0;
  v_index = 0;
  val = 1;
  PDC_in_transmission = false;
}

void PDCreceive::checkIR(IRrecv irrecv, decode_results results) {
  if (irrecv.decode(&results)) {  
//    Serial.print("got something"); Serial.println(val);
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
}

//translate the code to a section on the PDC or a number

int PDCreceive::translateCodes1(int key) {
   if(key == 0 && h_index >= 9)
     h_index = 0;

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

int PDCreceive::translateCodes2(int key) {  
   if (key == 10) {
     printTransmission();
     return 1;
   }
  length = key;
  Serial.print("from 2: ");
  Serial.println(key);
  return 3;
}

int PDCreceive::translateCodes3(int key) {
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
void PDCreceive::printTransmission() {
  
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

int PDCreceive::convertCodeToKey(long code) {
  for( int i=0; i < NUMCODES; i++) {
    if( code == irKeyCodes[i]) {
      return i; // found the key so return it
    }
  }
  return -1;
}

void PDCreceive::resetVariables() {
   for(int i=0;i<2;i++) {
     for(int j=0;j<maxNumberOfCodes;j++) {
       transmissionArray[i][j]=0;
     } 
   }
//  h_index = 0;                  // this can cause problems sometimes !!!!!!
  PDC_in_transmission = false;
  v_index=0;
//  val=1;
}
  
