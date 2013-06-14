int input;
int timeOfLast;
int pos;
byte the_byte;
byte max_bi;
volatile byte num_zeros;
const int ARR_SIZE = 400;
volatile int changes;
//volatile int capture_changes;
volatile byte index;
int ref_index;
volatile byte byte_index;
volatile byte storage_arr[ARR_SIZE];
volatile long spacing;

void setup() {
  Serial.begin(9600);
  max_bi = 0;
//  pinMode(8, INPUT);
  num_zeros = 0;
  index = 0;
  the_byte = 0;
//  capture_changes = 0;
  byte_index = 0;
  for(int i=0;i<ARR_SIZE;i++) {
    storage_arr[i] = 0;
  }
  spacing = 0;
  changes = 0;

  cli();
//set timer2 interrupt at 2kHz
  TCCR2A = 0;// set entire TCCR0A register to 0
  TCCR2B = 0;// same for TCCR0B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 100hz increments
  OCR2A = 160;// = (16*10^6) / (100*1024) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM22);
  // Set CS01 and CS00 bits for 64 prescaler
  //  TCCR0B |= (1 << CS01) | (1 << CS00);   
  TCCR2B |= (1 << CS22) | (1 << CS20); // 1024 prescale
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  
  
//  // setup for the input capture interrupt
//  TCCR1A = _BV(WGM10) | _BV(WGM11);             // Timer 1 is Phase-correct 10-bit PWM. 
//  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // div 8 clock prescaler to give .5us ticks on 16mhz clock
//  TCCR1B |= _BV(ICES1);                         // enable input capture
//  TIMSK1 =  _BV(ICIE1);                         // enable input capture interrupt for timer 1

sei();
}

void loop() {

  if(digitalRead(7) && changes > 10 || changes > 3100) {
    ref_index = index;
    index = 0;
    Serial.print(changes); Serial.print(", "); Serial.print(ref_index); Serial.print(": ");
    
    for(int i=0;i<ref_index;i++) {
      the_byte = storage_arr[i];
      if(the_byte < 128) {
        Serial.print(0);
        if(the_byte < 64) {
          Serial.print(0);
          if(the_byte < 32) {
            Serial.print(0);
            if(the_byte < 16) {
              Serial.print(0);
              if(the_byte < 8) {
                Serial.print(0);
                if(the_byte < 4) {
                  Serial.print(0);
                  if(the_byte < 2)
                    Serial.print(0);
                }
              }
            }
          }
        }
      }
//    Serial.print(changes);Serial.print(": ");Serial.println(spacing, BIN);
//      if
      Serial.print(storage_arr[i], BIN); Serial.print(" ");
    }
    Serial.println();
    byte_index = 0;
    index = 0;
//    capture_changes = 0;
    changes = 0;
  }
  
}

ISR(TIMER2_COMPA_vect) {
// if one of the last four pulses was true or current transmission is true 
//  if(!(byte_index > 6 && storage_arr[index] == 0) || (PINB & B0000001)){  
  if((PINB & B0000001) || num_zeros < 250) {
    storage_arr[index] = (storage_arr[index] << 1) | (PINB & B0000001);
    // increment byte_index and make sure its less than 8
    if(PINB & B0000001)
      num_zeros=0;
    else
      num_zeros++;
    byte_index++; byte_index &= B111;     
    if(!byte_index) index++; //byte_index was just reset so increment the array index
    changes++;
  }
}

//ISR(TIMER1_CAPT_vect) {
//   if(bit_is_set(TCCR1B ,ICES1)){  // was rising edge detected? - last bit was 0  
//      storage_arr[index]  = (storage_arr[index] << 1);
//   } else {                         // falling edge was detected 
//      storage_arr[index]  = (storage_arr[index] << 1) | 1;
//   }     
//   TCCR1B ^= _BV(ICES1);                 // toggle bit value to trigger on the other edge
////   capture_changes++;
//   changes++;
//   
//    // increment byte_index and make sure its <= 8
//    byte_index++; 
//    byte_index &= B111; 
//    if(!byte_index) index++;  //byte_index was just reset so increment the array index
//   TCNT2  = 0; // reset timer2 counter
//}
