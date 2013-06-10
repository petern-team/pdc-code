/*
This sketch will use timer and pin interrupts to sample a signal on one of
the pins. The signal will start with a high voltage for three divisions 
followed by a binary message


References:
http://www.instructables.com/id/Arduino-Timer-Interrupts/
*/

void setup(){

cli();//stop interrupts

//set timer0 interrupt at 10kHz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 25;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  
  sei();
  
}

