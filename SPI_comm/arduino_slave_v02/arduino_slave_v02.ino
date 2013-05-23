/*
Teensy-Arduino SPI communication, prototype v01

arduino slave code
*/

#include <SPI.h>

int SPI_in[100];
volatile byte pos;
volatile boolean SPI_complete;

boolean button_1pressed;
boolean button_2pressed;
int sensorValue;

void setup() {
  Serial.begin(9600);
  SPI.attachInterrupt();
  pinMode(MISO, OUTPUT);
  
  SPCR |= _BV(SPE);
  
  pos=0;
  SPI_complete = false;
}

// SPI interrupt routine
// information will be received in triplets of button_1.pressed, button_2.pressed,
// and sensorValue. At the end of each triplet will be a sentinel (-1)
ISR (SPI_STC_vect)
{
  char input = SPDR;
  
  // add to buffer if room
  if (pos < sizeof(SPI_in)) {
    SPI_in[pos] = input+0;
    
    // example: newline means time to process buffer
    if (SPI_in[pos] == -1)
      SPI_complete = true;
      
    pos++;
  }  else {
    Serial.println("Error: buffer full");
  }
}

void loop() {
  if(SPI_complete) {
    button_1pressed = SPI_in[0];
    button_2pressed = SPI_in[1];
    sensorValue = SPI_in[2];
    for(int i=pos; i>0; i--) {
      SPI_in[--pos] = 0;
    }
    SPI_complete = false;
    Serial.print("sensor value: "); Serial.println(sensorValue);
  }

}
  
  

