/*
This sketch decodes IR messages written with category/ time pairs.
Then it prints an opening sequence to the computer, followed by a 
character number (sometimes) and all the number pairs.
 */

#include <IRremote.h>           // IR remote control library

const int irReceivePin = 10;     // pin connected to IR detector output
const int numberOfKeys = 11;     //  how many keys you want to learn
const int maxNumberOfCodes = 100;
const String transmission_sequence = "abcdefg";

int length;
int val = 1;

int redLED = 1;
int greenLED = 2;

long transmissionArray[2][maxNumberOfCodes];
int index = 0;
int v_index = 0;

IRsend irsend;

long irKeyCodes[numberOfKeys] = {
    0x18E738A7, //from here down are all numbers, this is 0
    0x18E748A7,     // 1
    0x18E768A7,    // 2
    0x18E778A7,       // 3
    0x18E788A7,     // 4
    0x18E798A7,      // 5
    0x18E718B7,       // 6
    0x18E728B7,      // 7
    0x18E738B7,      // 8
    0x18E758B7,     // 9
    0x18E7C8E7 // end of transmission  
};

IRrecv irrecv(irReceivePin);    // create the IR library
decode_results results;         // IR data goes here

boolean in_transmission;
int timeOfLast= -3000;

void setup()
{
  
  for(int i=0;i<2;i++) {
   for(int j=0;j<maxNumberOfCodes;j++) {
    transmissionArray[i][j]=0;
   } 
  }

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  Serial.begin(9600);

  pinMode(irReceivePin, INPUT);

  irrecv.enableIRIn();              // Start the IR receiver
 
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    
    in_transmission = false;

}

void loop()
{

  if(in_transmission == false) {
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, HIGH);
  } else {
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, HIGH);
  }

 
  long key;
    

  if (irrecv.decode(&results))
  {
    
    Serial.println("got something");
    in_transmission = true;
    Serial.println(results.value, HEX);
    // here if data is received
    irrecv.resume();
    
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);

//    showReceivedData();
    
    key = convertCodeToKey(results.value);
    
    switch (val) {
    case 1: 
      val = translateCodes1(key);
      break;
 
    case 2: 
      val = translateCodes2(key);
      break;
    
    case 3:
      val = translateCodes3(key);
      break;
    }
    timeOfLast = millis();
  }
  
  if(millis() - timeOfLast > 3000) {
    timeOfLast = millis();
    resetVariables();
  } 
}

//translate the code to a section on the PDC or a number

int translateCodes1(long key) {
  
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
  

   if (key == 10) {
     printTransmission();
     return 1;
   } else {
     Serial.print("from 1: ");
     Serial.println(key);
     transmissionArray[0][index] = key;
     return 2;
   }
     
}

int translateCodes2(long key) {  
     if (key == 10) {
     printTransmission();
     return 1;
   }
  length = key;
       Serial.print("from 2: ");
     Serial.println(key);
  return 3;
}

int translateCodes3(long key) {
  long number = 0;
  
     if (key == 10) {
       printTransmission();
     return 1;
   }
  
  if (v_index < length) {
    
    // if the length is four multiply by 1000, if its 3 multiply by 100, etc. then do length = length -1
    // this function should never be called when length = 0, length should always be redefined before that point
    

    number = key;
    for (int i=0; i<v_index; i++) {
      key *= 10;
    }
    transmissionArray[1][index] += key;
    v_index++;
    
    if (v_index == length) {
      index++;
      v_index = 0;
      return 1;
    } else {
      return 3;
    }   
  } else {
    Serial.println("Error in 3");
    length = 0;
    v_index = 0;
    return 1;

  }
}


//print the array when the end of transmission code is received

void printTransmission() {
  
   Serial.println("print transmission called");
   Serial.print("index = "); Serial.println(index);
   delay(10);
  
  for (int i=0; i<index; i++) {
    Keyboard.print(transmissionArray[0][i]);
    delay(10);
    Keyboard.print(",");
    delay(10);
    Keyboard.print(transmissionArray[1][i]);
    delay(10);
    if (i == index-1) {
      Keyboard.print(":");
      delay(10);
    } else {
      Keyboard.print(";");
      delay(10);
    }
    
  }
}

void resetVariables() {
   for(int i=0;i<2;i++) {
   for(int j=0;j<maxNumberOfCodes;j++) {
    transmissionArray[i][j]=0;
   } 
  }
  
  index = 0;                  // this can cause problems sometimes !!!!!!
  in_transmission = false;
  v_index=0;
  val=1;
}

//counts the number of characters in transmissionArray, plus commas and semicolons

int countCharacters(int num_cols)
{
  int num_chars=0;
  num_chars=num_cols*4;    // two numbers per column, one comma/semicolon/colon per number
  return num_chars;
}

/*
 * converts a remote protocol code to a logical key code 
 * (or -1 if no digit received)
 */

int convertCodeToKey(long code)
{

  for( int i=0; i < numberOfKeys; i++)
  {

    if( code == irKeyCodes[i])
    {
      return i; // found the key so return it
    }
  }
  return -1;
}
