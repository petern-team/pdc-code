// Written by Nick Gammon
// April 2011

#include <SPI.h>

// what to do with incoming data
byte command = 0;
int sample_times[] = {1,2,3,4,5,6,7,8};
int pos = 0;

void setup ()
{
  Serial.begin(9600);
//  SPI.attachInterrupt();
  
  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);

  // turn on SPI in slave mode
  SPCR |= _BV(SPE);

}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
  byte c = SPDR;
  SPDR = c+sample_times[pos];
  Serial.println(pos);
  pos++;
  if(pos == 8)
    pos = 0;
 
//  switch (command)
//  {
//  // no command? then this is the command
//  case 0:
//    command = c;
//    SPDR = 0;
//    break;
//    
//  // add to incoming byte, return result
//  case 'a':
//    SPDR = c + 15;  // add 15
//    break;
//    
//  // subtract from incoming byte, return result
//  case 's':
//    SPDR = c - 8;  // subtract 8
//    break;
//
//  } // end of switch

//}  // end of interrupt service routine (ISR) SPI_STC_vect

void loop (void)
{
  checkSPI();
  // if SPI not active, clear current command
  if (digitalRead (SS) == HIGH)
    pos = 0;
}  // end of loop

void checkSPI()
{
  if(digitalRead(SS) == LOW) {
    byte c = SPDR;
    SPDR = c+sample_times[pos];
    Serial.println(pos);
    pos++;
    if(pos == 8)
      pos = 0;
  }
}
