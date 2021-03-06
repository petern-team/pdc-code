/*
Teensy-Arduino SPI communication, prototype v01

teensy master code
*/

#include <SPI.h>

const int SSPIN = 0;
boolean button1_pressed;
boolean button2_pressed;
int potenti_state;

void setup() {
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
}

void loop() 
{
  char output[] = {0,1,2,-1};
  digitalWrite(SSPIN, LOW);
  
  for(int i=0;i<(sizeof(output)/2);i++) {
    Serial.println(output[i]);
    SPI.transfer(output[i]);
  }
  
  digitalWrite(SSPIN, HIGH);
  
  delay(1000);
}
