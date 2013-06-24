#include "audioCommPin.h"

boolean transmission_comp;

void setup() {
  Serial.begin(9600);
  transmission_comp = false;
  resetStorage();
  initAudioPin();
}

void loop() {
  
  transmission_comp = checkTransmission();
    
  if(transmission_comp) {
    parseArray();
    printArray();
    resetStorage();
    resetChars();
  }
}
      
