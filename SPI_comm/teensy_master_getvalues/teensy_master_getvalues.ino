/*
Teensy-Arduino SPI communication, prototype v01

teensy master code
*/

#include <SPI.h>

const int SSPIN = 0;
boolean button1_pressed;
boolean button2_pressed;
int potenti_state;
byte sample_times[8];

void setup() {
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
}

void loop() 
{

  digitalWrite(SSPIN, LOW);
  
  getValue();
  
  for(int i=0;i<8;i++) {
    sample_times[i] = getValue();
    Serial.print(sample_times[i]+0); Serial.print(", ");
//    delay(1);
  }
    digitalWrite(SSPIN, HIGH);
  
//  for(int i=0;i<8;i++) {
//    Serial.print(sample_times[i]+0); Serial.print(", ");
//  }
  Serial.println();

  delay(3000);
}

byte getValue()
{
  byte from_slave = SPI.transfer(0);
  delayMicroseconds(20);
  return from_slave;
}
