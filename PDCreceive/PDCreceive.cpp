#include "PDCreceive.h"
#include <IRremote.h>

PDCreceive::PDCreceive() {
    pinMode(RECEIVEPIN, INPUT);
    //  irrecv.enableIRIn();              // Start the IR receiver
    
    for(int i=0;i<100;i++) {
        transmissionArray[i]='-';
    }
    index = 0;
    PDC_in_transmission = false;
}

//PDCreceive::PDCreceive(int receive_pin) {
//    pinMode(receive_pin, INPUT);
//    //  irrecv.enableIRIn();              // Start the IR receiver
//    
//    for(int i=0;i<2;i++) {
//        for(int j=0;j<maxNumberOfCodes;j++) {
//            transmissionArray[i][j]=0;
//        }
//    }
//    h_index = 0;
//    v_index = 0;
//    val = 1;
//    PDC_in_transmission = false;
//}

void PDCreceive::checkIR(IRrecv irrecv, decode_results results) {
    if (irrecv.decode(&results)) {
        //    Serial.print("got something"); Serial.println(val);
        PDC_in_transmission = true;
        Serial.println(results.value, HEX);
        // here if data is received
        irrecv.resume();
        
        //    showReceivedData();
        
        key = convertCodeToKey(results.value);
        storeData(key);
    }
}

//print the array when the end of transmission code is received
void PDCreceive::printTransmission() {
    
    Serial.println("print transmission called");
    Serial.print("index = "); Serial.println(index);
    //   delay(10);
    
    for (int i=0; i<index; i++) {
        Keyboard.print(transmissionArray[i]);
    }
    resetVariables();
}

void PDCreceive::storeData(int key) {
    if(key < 10) {
        transmissionArray[index] = '0'+key;
    } else {
        Serial.print("switchCase: "); Serial.println(key);
        switch (key) {
            case 10:
                transmissionArray[index] = ',';
                break;
            case 11:
                transmissionArray[index] = ';';
                break;
            case 12:
                transmissionArray[index] = ':';
                printTransmission();
                return;
        }
    }
    index++;
}

int PDCreceive::convertCodeToKey(long code) {
    for( int i=0; i < NUMCODES; i++) {
        if( code == irKeyCodes[i]) {
            return i; // found the key so return it
        }
    }
    return -1;
}

void PDCreceive::resetVariables() {
    for(int i=0;i<100;i++) {
        transmissionArray[i]=0;
    }
    index = 0;                  // this can cause problems sometimes !!!!!!
    PDC_in_transmission = false;
    v_index=0;
    val=1;
}
