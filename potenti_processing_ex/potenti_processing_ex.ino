// test processing graphing with a potentiometer

const int sensorPin = A0;
int sensorValue;
boolean in_transmit;

void setup() {
  in_transmit = true;
  pinMode(sensorPin, INPUT);
//  pinMode(8, INPUT);
  Serial.begin(9600);
//  delay(500);
//  byte is_true = B00001000 << 4;
//  Serial.println(is_true);
//  is_true = B00010000 << 4;
//  Serial.println(is_true, BIN);
//  is_true = B00001000 & B1111;
//  Serial.println((is_true), BIN);
//  delay(4000);
}

void loop() {
  if(in_transmit) {
    sensorValue = analogRead(sensorPin);
    Serial.println(sensorValue); //PIND & (1<<2));
    delay(50);
  }
  if(millis() > 20000 && in_transmit) {
    Serial.println('\n');
    in_transmit = false;
  }
}
