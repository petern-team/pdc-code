/* this program will take raw codes send to teensy and translate them into keystrokes */

#include <IRremote.h>

const int irReceivePin = 10;

const int NUM_CODES = 19;
byte raw_codes[NUM_CODES] = {0x501,0xD02,0x503,0xD04,0x505,0xD06,
            0x507,0xD08,0x509,0xD00,0x520,0xD21,0x511,0xD10,0x53B,0xD31,0x50C,0xD32,0x535};
byte constants = INPUT;              

String code_IDs[NUM_CODES] = {"1","2","3","4","5","6","7","8","9","0",
              "up","down","left","right","enter","switch","tab","shift","caps"};
int length;

IRrecv irrecv(irReceivePin);
decode_results results;

IRsend irsend;

void setup() {
  Serial.begin(9600);
  Serial.println(constants, HEX);
  
  pinMode(irReceivePin, constants);
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
