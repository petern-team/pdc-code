
// simple sketch to continually sample the input from a digital pin into 
// an array, until a string of 8 zeros is received

#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

const int inPin = A3;
const int SIZE = 1000;
byte data_collect[SIZE];
int pos;
int ref_pos;
byte val;
boolean started;
boolean done;
byte last_byte[8];
int initial_t;

void setup() {
  
  #if FASTADC
  // set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
  #endif
  
  Serial.begin(9600);
  pinMode(inPin, INPUT);
  pos = 0;
  ref_pos = 0;
  started = true;
  done = false;
  delay(2000);
  initial_t = millis();
}

void loop() {
  val = map(analogRead(inPin),0,1023,0,255);
//  if(!started && !done && val>0) {
////    Serial.println("started");
//    started = true;
//  }
  if(ref_pos > 7)
    ref_pos -= 8;
  if(started && !done) {
    data_collect[pos] = val;
    last_byte[ref_pos] = val;
    pos++;
    checkIfDone();
  }

  if(done && started) {
//    Serial.print("done: "); Serial.print(pos+1); Serial.println("bytes stored");
    printArray();
    started = false;
  }
}

// if last_byte is all zeros, then done =true

void checkIfDone() {
//  done = true;
//  for(int i=0;i<8;i++) {
//    if(last_byte[i]>0)
//      done = false;
//  }
  if(pos >= SIZE-1) {
    done = true;
  }
}

void printArray() {
  for(int i=0;i<pos;i++) {
    Serial.println(data_collect[i]);
//    delay(10);
  }
  Serial.println('\n');
  Serial.print("done: "); Serial.println(millis()-initial_t);
}
