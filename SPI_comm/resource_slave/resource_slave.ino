
//#include "pins_arduino.h"
#include <SPI.h>

char buf [100];
volatile byte pos;
volatile boolean process_it;

void setup (void)
{
  Serial.begin (9600);   // debugging
  SPI.attachInterrupt();

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);
  
  pos = 0;
  process_it = false;
}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
byte c = SPDR;
  
  // add to buffer if room
  if (pos < sizeof buf)
    {
    //  pos++;
    buf [pos++] = c;
    
    // example: newline means time to process buffer
    if (c == '\n')
      process_it = true;
      
    }  // end of room available
}

// main loop - wait for flag set in interrupt routine
void loop () {
  if (process_it)
    {
    buf [pos] = 0;  
    buf [pos-1]= 0;
   Serial.println (buf);
    pos = 0;
    process_it = false;
    }  // end of flag set
    
}  // end of loop

