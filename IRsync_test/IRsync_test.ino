#include <PDCsend.h>
#include <PDCreceive.h>
#include <IRremote.h>

IRrecv irrecv(9);
decode_results results;
PDCsend pdcSend;
PDCreceive pdcReceive;

void setup() {
  Serial.begin(9600);
  delay(4000);
  Serial.println("sending codes");
  pdcSend.sendSyncCode(73201);
  irrecv.enableIRIn();
}

void loop() {
  pdcReceive.checkIR(irrecv, results);
  if(pdcReceive.transmission_complete) {
    if(pdcReceive.PDC_sync) {
      Serial.println("sending sync codes");
//      for(int i=0;i<5;i++) {
//        DTDsend.sendSyncCode(PRODUCT_ID);
//        delay(50);
//      }
      pdcReceive.PDC_sync = false;
    } else {
      pdcReceive.printTransmission();
    }
    pdcReceive.resetVariables();
  }lt
}
