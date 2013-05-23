
//#include "pins_arduino.h"

void setup ()
{
  // Put SCK, MOSI, SS pins into output mode
  // also put SCK, MOSI into LOW state, and SS into HIGH state.
  // Then put SPI hardware into Master mode and turn SPI on
  SPI.begin ();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  
}  // end of setup

void loop ()
{

  char c;

  // enable Slave Select
  digitalWrite(0, LOW);    // SS is pin 0

  // send test string
  for (const char * p = "Hello, world!\n" ; c = *p; p++)
    SPI.transfer (c);

  // disable Slave Select
  digitalWrite(0, HIGH);

  delay (1000);  // 1 seconds delay 
}  // end of loop
