const byte HEADER = B100011;
const byte TAIL = B111111;

#define SEARCH8 255
#define SEARCH12 4095
#define SEARCH16 65535

int input;
int timeOfLast;
int pos;
byte the_byte;
byte max_bi;
volatile byte num_zeros;
const int ARR_SIZE = 400;
volatile int changes;
volatile int index;
int ref_index;
volatile byte byte_index;

volatile boolean odd_array;
volatile byte *stagnant_data;
volatile unsigned short buffer;
volatile byte storage_arr[ARR_SIZE];
char char_arr[(ARR_SIZE*2)/5];        // adjust size to account for storage strategy
volatile int condensed_index;
int char_index;

void setup() {
  Serial.begin(9600);
//  raw_data = raw_data_even;
//  odd_array = false;
  buffer = 1;

  num_zeros = 0;
  index = 0;
  condensed_index = 0;
  char_index = 0;
  byte_index = 0;

  for(int i=0;i<ARR_SIZE/2;i++) {
    storage_arr[i] = 0;
  }
  changes = 0;

  cli();
//set timer2 interrupt at 1.6kHz
  TCCR2A = 0;// set entire TCCR0A register to 0
  TCCR2B = 0;// same for TCCR0B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 1600hz increments
  OCR2A = 156;// = (16*10^6) / (1600*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set prescaler
  //  TCCR2B |= (1 << CS20);  // no prescaler
  //  TCCR2B |= (1 << CS21);  // 8 prescaler
  TCCR2B |= (1 << CS22);  // 64 prescaler 
//  //  TCCR2B |= (1 << CS21) | (1 << CS20);  // 256 prescaler - not for timer2
//  TCCR2B |= (1 << CS22) | (1 << CS20); // 1024 prescaler - not for timer2
  
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  
  
  // setup for the input capture interrupt
  TCCR1A = _BV(WGM10) | _BV(WGM11);             // Timer 1 is Phase-correct 10-bit PWM. 
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // div 8 clock prescaler to give .5us ticks on 16mhz clock
  TCCR1B |= _BV(ICES1);                         // enable input capture
  TIMSK1 =  _BV(ICIE1);                         // enable input capture interrupt for timer 1

sei();
}

void loop() {
    
  if(digitalRead(7) && changes > 10) {
    Serial.print("changes: "); Serial.println(changes);
//    Serial.println(condensed_index);
//    ref_index = index;
//    stagnant_data = raw_data;
//    index = 0;
//    byte_index = 0;
    changes = 0;
//    storeArray(ref_index);
    for(int i=0;i<condensed_index;i++) {
      Serial.print(storage_arr[i]); Serial.print(", ");
    }
    Serial.println();
    parseArray();
    printArray();
    condensed_index = 0;
  }
}


ISR(TIMER2_COMPA_vect) {
// if one of the last four pulses was true or current transmission is true  
  if((PINB & B0000001) || num_zeros < 75) {
    buffer = ((buffer << 1) | (PINB & B00000001)) & SEARCH12;
    
    if(PINB & B0000001)
      num_zeros=0;
    else
      num_zeros++;
      
    if(buffer == 0 || buffer == SEARCH12) {
      storage_arr[condensed_index] = buffer & 1;
      condensed_index++;
      buffer ^= 1;
    }
//    byte_index++; byte_index &= B111;     
//    if(!byte_index) 
//      index++; //byte_index was just reset so increment the array index
    changes++;
  }
}

ISR(TIMER1_CAPT_vect) {
   if(!bit_is_set(TCCR1B ,ICES1)){  // was falling edge detected? - last bit was 1  
      buffer = buffer << 1 & SEARCH12;
   } else {                         // rising edge was detected
      buffer = (buffer << 1) | 1 & SEARCH12;
   }     
   TCCR1B ^= _BV(ICES1);                 // toggle bit value to trigger on the other edge
   changes++;
   num_zeros = 0;
    if(buffer == 0 || buffer == SEARCH12) {
      storage_arr[condensed_index] = buffer & 1;
      condensed_index++;
      buffer ^= 1;
    }
   
    // increment byte_index and make sure its <= 8
//    byte_index++; 
//    byte_index &= B111; 
//    if(!byte_index) index++;  //byte_index was just reset so increment the array index
   TCNT2  = 0; // reset timer2 counter
}
  
void parseArray() {
  char the_char = 0;
  int search_index = 0;
  
  while(search_index <= condensed_index) {
    findBuffer(&search_index, HEADER);
//    Serial.println(search_index);
    if(search_index == -1) {
//      Serial.println();
//      Serial.println("Error: no header found");
      return;
    }
    the_char = getChar(search_index);
    char_arr[char_index] = the_char;
    char_index++;
    Serial.println(char_arr[char_index-1]+0);
    search_index += 8;
    findBuffer(&search_index, TAIL);
//    Serial.println(search_index);
    if(search_index == -1) {
//      Serial.println("Error: no tail found");
      return;
    }
    while(storage_arr[search_index] == 0)
      search_index++;
  } 
//  Serial.println();
}

void printArray() {
  for(int i=0;i<char_index;i++) {
    Serial.print(char_arr[i]);
  }
  Serial.println();
}

void findBuffer(int *index, byte search) {
  byte to_compare = 0;
  for(int i=*index;i<condensed_index;i++) {
    to_compare <<= 1;
    to_compare += storage_arr[i];  // add 0 or 1
    to_compare &= B111111; //keep only the 6 least significant bytes
    if(to_compare == search) {    // header was found
      *index = i+1;
      return;
    }
  }
  *index = -1;
}

char getChar(int start) {
  byte the_char = 0;
  for(int i=0;i<8;i++) {
    boolean the_bit = !storage_arr[start+i];
    the_char += pow(2*the_bit,7-i);
  }
  return the_char+0;
}

int pow(int base, int power) {
  int ans=1;
  if (base == 0)
    return 0;
  for(int i=0;i<power;i++) {
    ans *= base;
  }
  return ans;
}
      
