#include <PDCsend.h>

// PDCsend example


long unsigned time_array[] = {13,15,0,0,789,44,120,0};

const int PRODUCT_ID = 73201; //"=> PDC01"
const int SEND_TIMES_ID = 001;

PDCsend myPDC;

void setup() {
  Serial.begin(9600);
  Serial.println("creating array");
  myPDC.createArray(PRODUCT_ID, SEND_TIMES_ID, time_array);
  Serial.println("sending array");
  myPDC.sendArray();
}

void loop() {}
