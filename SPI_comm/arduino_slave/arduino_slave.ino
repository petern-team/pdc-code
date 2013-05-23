/*
Teensy-Arduino SPI communication, prototype v01

arduino slave code
*/

#include <SPI.h>

char SPI_in[100];
volatile byte pos;
volatile boolean SPI_complete;

void setup() {
  Serial.begin(9600);
  SPI.attachInterrupt();
  pinMode(MISO, OUTPUT);
  
  SPCR |= _BV(SPE);
  
  pos=0;
  SPI_complete = false;
}

// SPI interrupt routine
ISR (SPI_STC_vect)
{
byte input = SPDR;
  
  // add to buffer if room
  if (pos < sizeof(SPI_in))
    {
    //  pos++;
    SPI_in[pos++] = input;
    
    // example: newline means time to process buffer
    if (input == '\n')
      SPI_complete = true;
      
    }  else {
      Serial.println("Error: buffer full");
    }
}

void loop() {
  if(SPI_complete) {
    SPI_in[--pos] = 0;
    Serial.println(SPI_in);
    for(pos;pos>=0;pos--) {
      SPI_in[pos] = 0;
    }
  }
}
  
  

