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
    transmission_complete = false;
    PDC_sync = false;
}

PDCreceive::PDCreceive(long product) {
    pinMode(RECEIVEPIN, INPUT);
    //  irrecv.enableIRIn();              // Start the IR receiver
    
    for(int i=0;i<100;i++) {
        transmissionArray[i]='-';
    }
    index = 0;
    PDC_in_transmission = false;
    transmission_complete = false;
    PDC_sync = false;
    PRODUCT_ID = product;
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
//        Serial.println("got something");
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
    
//    Serial.println("print transmission called");
//    Serial.print("index = "); Serial.println(index);
//    //   delay(10);
//    if(syncCodeRecvd()) {
//        PDC_sync = true;
//    } else {
//    
        for (int i=0; i<index; i++) {
            Serial.print(transmissionArray[i]);
        }
//    }
}

//check if the transmission received is a valid product ID
//bool PDCreceive::syncCodeRecvd() {
//    String my_id = "";
//    if(index > 7)
//        return false;
//    for(int i=0;i<index;i++) {
//        my_id += transmissionArray[i];
//        if(transmissionArray[i] == ',' || transmissionArray[i] == ';')
//            return false;
//    }
////    if(my_id == String(PRODUCT_ID)+':') {
////        Serial.print(my_id); Serial.print(": "); Serial.println(String(PRODUCT_ID)+':');
////        return false;
////    } else {
//       return true;
////    }
//}

// storeData replaced the translatedCode functions, it simply translates the index
// of an IR code into a character and changes transmission_complete to true when it
// finds a colon

void PDCreceive::storeData(int key) {
    if(key == -1)
        transmissionArray[index] = '/';
    else
        transmissionArray[index] = keyIndex[key];
    
    if(key == 12) {
        transmission_complete = true;
        checkTransmission();
        return;
    }
//    if(key < 10) {
//        transmissionArray[index] = '0'+key;
//    } else {
//        Serial.print("switchCase: "); Serial.println(key);
//        switch (key) {
//            case 10:
//                transmissionArray[index] = ',';
//                break;
//            case 11:
//                transmissionArray[index] = ';';
//                break;
//            case 12:
//                transmissionArray[index] = ':';
//                transmission_complete = true;
////                printTransmission();
//                
//                return;
//        }
//    }
    index++;
}

// checkTransmission checks to make sure there's a legitimate header to the transmission
// and that the checkSum works out

// MAKE SURE THIS WORKS

void PDCreceive::checkTransmission() {
    int chk_indx;
    long incoming_id = 0;
    int transmission_id = 0;
    int checksum = 0;
    int num_chars = 0;
    int char_to_int;
    for(int i=0; i<5; i++) {
        incoming_id *= 10;
        incoming_id += transmissionArray[i]-'0';//(transmissionArray[i]-'0')*(pow(10, i));
    }
    Serial.print("product id: "); Serial.println(incoming_id);
    
    if(transmissionArray[5] == ':') {
        PDC_sync = true;
        return;
    }
    
    if(transmissionArray[5] != ',') {
        Serial.println("Error: invalid product id");
    }
    
    for(chk_indx=6;transmissionArray[chk_indx] != ',';chk_indx++) {
        transmission_id *= 10;
        transmission_id += transmissionArray[chk_indx]-'0';
    }
    Serial.println(transmission_id);

    for(++chk_indx;transmissionArray[chk_indx] != ';';chk_indx++) {
        checksum *= 10;
        checksum += transmissionArray[chk_indx]-'0';//(transmissionArray[chk_indx]-'0')*(pow(10, chk_indx-initial));
    }
    Serial.print("check sum "); Serial.println(checksum);
    
    for(++chk_indx; transmissionArray[chk_indx] != ':'; chk_indx++) {
        char_to_int = transmissionArray[chk_indx]-'0';
        if(char_to_int == -1)
            Serial.println("Error: invalid IR code");
        else if(char_to_int >= 0 && char_to_int < 10 )
            num_chars++;
            
    }
    Serial.print("num_chars: "); Serial.println(num_chars);
    if(num_chars != checksum)
        Serial.println("Error: wrong number of codes received");

    
}

// getChar
char PDCreceive::getChar(int index) {
    return transmissionArray[index];
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
    transmission_complete = false;
}

long PDCreceive::pow(int base, int power) {
    long ans = 1;
    for(int i=0;i<power;i++) {
        ans *= base;
    }
    return ans;
}
