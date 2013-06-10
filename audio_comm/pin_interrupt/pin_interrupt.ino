int input;
int timeOfLast;
int pos;
int changes;
//int spacing[100];
volatile long spacing;
volatile boolean val;

void setup() {
  Serial.begin(9600);
  pinMode(8, INPUT);
  pinMode(11, OUTPUT);
  analogWrite(11, 10);
  //  pos = 0;
  //  timeOfLast = 0;
  spacing = 0;
  changes = 0;
  val = false;

  cli();
//set timer0 interrupt at 10kHz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 32;// = (16*10^6) / (10000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
//  TCCR0B |= (1 << CS01) | (1 << CS00);   
TCCR0B |= (1 << CS02) | (1 << CS00); // 1024 prescale
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  
  
  // setup for the input capture interrupt
  TCCR1A = _BV(WGM10) | _BV(WGM11);             // Timer 1 is Phase-correct 10-bit PWM. 
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // div 8 clock prescaler to give .5us ticks on 16mhz clock
  TCCR1B |= _BV(ICES1);                         // enable input capture
  TIMSK1 =  _BV(ICIE1);                         // enable input capture interrupt for timer 1

sei();
}

void loop() {
//  input = (PINB & B000001);// >> 3;
//  delay(1000);
//  if(pos == 10) {
//    printArray();
//    pos = 0;
//  }
//Serial.println(input, BIN);//  >> 3));
//delay(1000);
  if(changes > 120) {
    Serial.println(spacing, BIN);
    changes = 0;
  }
}

ISR(TIMER0_COMPA_vect) {
  spacing = (spacing << 1) | (PINB & 0000001);
  changes++;
  TCNT0  = 0;
}

ISR(TIMER1_CAPT_vect) {
   if( bit_is_set(TCCR1B ,ICES1)){       // was rising edge detected? - last bit was 0  
      spacing  = (spacing << 1);
   } else {         // falling edge was detected 
      spacing  = (spacing << 1) | 1;
   }     
   TCCR1B ^= _BV(ICES1);                 // toggle bit value to trigger on the other edge
   changes++;
   TCNT0  = 0; // reset timer0 counter
}
