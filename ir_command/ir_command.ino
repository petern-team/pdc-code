/* this program will take raw codes send to teensy and translate them into keystrokes */

#include <IRremote.h>

const int irReceivePin = 10;

const int NUM_CODES = 16;
byte raw_codes[NUM_CODES] = {0x801,0x2,0x803,0x4,0x805,0x6,0x807,
                    0x8,0x809,0x0,0x82D,0x19,0x82C,0x2B,0x80C,0x38};

//String code_IDs[50] = {"1"
int length;

IRrecv irrecv(irReceivePin);
decode_results results;

IRsend irsend;

void setup() {
  Serial.begin(9600);
  
  pinMode(irReceivePin, INPUT);
  irrecv.enableIRIn();

}

void loop()

{

  long key;

  if (irrecv.decode(&results))
  {
    // here if data is received
    irrecv.resume();

    key = convertCodeToKey(results.value);

    if(key >= 0)
    {
      Serial.print("Got key ");
//      Serial.println(key);
      Serial.println(key);
      delay(50);
    }
  }
}

int convertCodeToKey(long code)
{

  for( int i=0; i < NUM_CODES; i++)
  {

    if( code == raw_codes[i])
    {
      return i; // found the key so return it
    }
  }
  return -1;
}
