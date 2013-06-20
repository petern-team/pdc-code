//#include "audioCommPin.h"
//
//// 
//
//
//void initAudioPin() {
//    cli();
//    //set timer2 interrupt at 1.6kHz
//    TCCR0A = 0;// set entire TCCR0A register to 0
//    TCCR0B = 0;// same for TCCR0B
//    TCNT0  = 0;//initialize counter value to 0
//    // set compare match register for 1600hz increments
//    OCR0A = 156;// = (16*10^6) / (1600*64) - 1 (must be <256)
//    // turn on CTC mode
//    TCCR0A |= (1 << WGM01);
//    // Set prescaler
//    //  TCCR2B |= (1 << CS20);  // no prescaler
//    //  TCCR2B |= (1 << CS21);  // 8 prescaler
//    TCCR0B |= (1 << CS01) | (1<<CS00);  // 64 prescaler
//    //  //  TCCR2B |= (1 << CS21) | (1 << CS20);  // 256 prescaler - not for timer2
//    //  TCCR2B |= (1 << CS22) | (1 << CS20); // 1024 prescaler - not for timer2
//    
//    // enable timer compare interrupt
//    TIMSK0 |= (1 << OCIE0A);
//    
//    
//    // setup for the input capture interrupt
//    TCCR1A = _BV(WGM10) | _BV(WGM11);             // Timer 1 is Phase-correct 10-bit PWM.
//    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // div 8 clock prescaler to give .5us ticks on 16mhz clock
//    TCCR1B |= _BV(ICES1);                         // enable input capture
//    TIMSK1 =  _BV(ICIE1);                         // enable input capture interrupt for timer 1
//    
//    sei();
//}
