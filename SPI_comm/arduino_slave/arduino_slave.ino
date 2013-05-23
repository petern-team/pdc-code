/*
Teensy-Arduino SPI communication, prototype v01

arduino slave code
*/

#include <SPI.h>

char SPI_in[100];
int data[100];
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
  char input = SPDR;
  
  // add to buffer if room
  if (pos < sizeof(data)) {
    data[pos] = input+0;
    
    // example: newline means time to process buffer
    if (data[pos] == -1)
      SPI_complete = true;
    pos++;
    }  else {
      Serial.println("Error: buffer full");
    }
}

void loop() {
  if(SPI_complete) {
   // SPI_in[--pos] = 0;
    //Serial.println(SPI_in
    for(int i=0;i<pos;i++) {
      Serial.print(data[i]); Serial.print(", ");
      data[i] = 0;
    }
    Serial.println();
    pos = 0;
    SPI_complete = false;
  }
}
  
  

