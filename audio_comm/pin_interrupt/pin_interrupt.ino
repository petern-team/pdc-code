const byte HEADER = B100011;
const byte TAIL = B111111;
const int ARR_SIZE = 400;

#define SEARCH2 3      //probs does not work
#define SEARCH3 7
#define SEARCH4 15
#define SEARCH6 63
#define SEARCH8 255
#define SEARCH12 4095
#define SEARCH16 65535

#define FINDGROUP SEARCH3


volatile byte num_zeros;
volatile int changes;
int last_state;
long time_of_last;
volatile int index;
volatile byte bit_index;

volatile unsigned short buffer;
volatile byte storage_arr[ARR_SIZE];
char char_arr[(ARR_SIZE*2)/5];        // adjust size to account for storage strategy
int char_index;

volatile unsigned long overflow_1600;
unsigned long seconds;

void setup() {
  Serial.begin(9600);
//  raw_data = raw_data_even;
//  odd_array = false;
  buffer = 1;

  num_zeros = 0;
  index = 0;
  index = 0;
  char_index = 0;
  bit_index = 0;
  last_state = 0;
  overflow_1600 = 0;

  for(int i=0;i<ARR_SIZE;i++) {
    storage_arr[i] = 0;
  }
  changes = 0;

  cli();
//set timer2 interrupt at 1.6kHz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 1600hz increments
  OCR0A = 156;// = (16*10^6) / (1600*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set prescaler
  //  TCCR2B |= (1 << CS20);  // no prescaler
  //  TCCR2B |= (1 << CS21);  // 8 prescaler
  TCCR0B |= (1 << CS01) | (1<<CS00);  // 64 prescaler 
//  //  TCCR2B |= (1 << CS21) | (1 << CS20);  // 256 prescaler - not for timer2
//  TCCR2B |= (1 << CS22) | (1 << CS20); // 1024 prescaler - not for timer2
  
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  
sei();
attachInterrupt(0,pinChange,CHANGE);
}

void loop() {
  seconds = overflow_1600/800;
  if(last_state != changes) {
    time_of_last = seconds;
  }
  last_state = changes;
    
  if(digitalRead(7) && changes > 10) {
//  if(seconds - time_of_last > 1 && changes > 100) { //index > 2) {
    changes = 0;
    Serial.print("index: "); Serial.println(index);
//    Serial.println(index);
    changes = 0;
    parseArray();
    printArray();
    index = 0;
    bit_index = 0;
  }
}


ISR(TIMER0_COMPA_vect) {
  overflow_1600++;
// if one of the last four pulses was true or current transmission is true  
  if((PIND & B0000100) || num_zeros < 75) {
    buffer = ((buffer << 1) | ((PIND & B00000100)) >> 2) & FINDGROUP;
    
    if(PIND & B0000100)
      num_zeros=0;
    else
      num_zeros++;
      
    if(buffer == 0 || buffer == FINDGROUP) {
      storage_arr[index] = (storage_arr[index] << 1) | (buffer & 1);
      buffer ^= 1;
      bit_index++; 
      bit_index &= B111;     
      if(!bit_index) index++; //bit_index was just reset so increment the array index
    }
    changes++;
  }
}
void pinChange() {
  buffer = ((buffer << 1) | ((PIND & B0000100) >> 2)) & FINDGROUP;
//   if(PIND & B0000100)
//     one_changes++;
//   else
//     zero_changes++;
 num_zeros = 0;
 changes++;
   
 if(buffer == 0 || buffer == FINDGROUP) {
    storage_arr[index] = (storage_arr[index] << 1) | (buffer & 1);
    buffer ^= 1;
    
    // increment bit_index and make sure its <= 8
    bit_index++; 
    bit_index &= B111; 
    if(!bit_index) index++;  //bit_index was just reset so increment the array index
  }
   
  TCNT0 = 0;
}
  
void parseArray() {
  char the_char = 0;
  int byte_index = 0;
  byte search_bit_index = 0;
  
  while(byte_index <= index) {
    findBuffer(&byte_index, &search_bit_index, HEADER);
//    Serial.print("header: "); Serial.print(byte_index); Serial.print(","); Serial.println(search_bit_index);
    if(byte_index == -1) {
//      Serial.println();
//      Serial.println("Error: no header found");
      return;
    }
    the_char = getChar(byte_index, search_bit_index);
    char_arr[char_index] = the_char;
    char_index++;
//    Serial.print("ascii char: "); Serial.println(the_char+0);
    byte_index++;
    findBuffer(&byte_index, &search_bit_index, TAIL);
//    Serial.println(search_index);
    if(byte_index == -1) {
//      Serial.println("Error: no tail found");
      return;
    }
    
    byte the_bit = 0;
    while(!((storage_arr[byte_index] << search_bit_index) & B10000000)) {
      search_bit_index++;
      search_bit_index &= B111; 
      if(!search_bit_index) byte_index++;  //was just reset so increment the array index
    } 
  }
//  Serial.println();
}

void printArray() {
  for(int i=0;i<char_index;i++) {
    Serial.print(char_arr[i]);
  }
  Serial.println();
}

void findBuffer(int *start_byte, byte *start_bit, byte search) {
  byte to_compare = 0;
  byte the_bit = 0;
  
  for(int i=*start_byte;i<index;i++) {
    for(int j=*start_bit;j<8;j++) {
      if(to_compare == search) {    // header or tail was found
        *start_byte = i;
        *start_bit = j;
        return;
      }
      the_bit = storage_arr[i] << j;
      the_bit >>= 7;
      
      to_compare <<= 1;
      to_compare += the_bit;  // add 0 or 1
      to_compare &= B111111; //keep only the 6 least significant bits
    }
    *start_bit = 0;
  }
  *start_byte = -1;
}

char getChar(int start_byte, byte start_bit) {
  byte the_char = 0;
  byte the_bit = 0;
  
  for(int i=0;i<8;i++) {
    the_bit = storage_arr[start_byte] << start_bit;
    the_bit >>= 7;
    
    boolean inverse_bit = !the_bit;
    the_char += pow(2*inverse_bit,7-i);
//    Serial.print("the_byte: "); Serial.println(storage_arr[start], BIN);
//    Serial.print("the_bit: "); Serial.println(the_bit);
    start_bit++;
    start_bit &= B111; 
    if(!start_bit) start_byte++;  //byte_index was just reset so increment the array index
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
      
