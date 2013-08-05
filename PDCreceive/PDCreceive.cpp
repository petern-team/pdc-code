#include "PDCreceive.h"
#include <IRremote.h>

PDCreceive::PDCreceive() {
    pinMode(RECEIVEPIN, INPUT);
    //  irrecv.enableIRIn();              // Start the IR receiver
    
    resetVariables();
}

PDCreceive::PDCreceive(long product) {
    pinMode(RECEIVEPIN, INPUT);
    //  irrecv.enableIRIn();              // Start the IR receiver
    
    PRODUCT_ID = product;
    resetVariables();
    
}

void PDCreceive::checkIR(IRrecv irrecv, decode_results results) {
    if (irrecv.decode(&results)) {
//        Serial.println("got something");
        PDC_in_transmission = true;
//        Serial.println(results.value, HEX);
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


// storeData replaced the translatedCode functions, it simply translates the index
// of an IR code into a character and changes transmission_complete to true when it
// finds a colon

void PDCreceive::storeData(int key) {
    if(key == -1) {
        
        // sometimes the first character of a transmission is weird
        if(index > 0)
           transmissionArray[index] = '/';
    } else
        transmissionArray[index] = keyIndex[key];
//    Serial.print(index); Serial.print(": "); Serial.println(transmissionArray[index]);
    if(key == 12) {
        transmission_complete = true;
//        Serial.print("transmission[5]= "); Serial.println(transmissionArray[5]);
        checkTransmission();
        return;
    }
    index++;
    if(index == 100) {
        checkTransmission();
        printTransmission();
        
        // think about replacing these with an overloaded function or something
        memset(transmissionArray, 0, sizeof(transmissionArray));
        index = 0;           
    }
}

// checkTransmission checks to make sure there's a legitimate header to the transmission
// and that the checkSum works out


void PDCreceive::checkTransmission() {
//    Serial.print("transmission[5]= "); Serial.println(transmissionArray[5]);
//    int chk_indx;
    long incoming_id = 0;
    for(int i=0; i<5; i++) {
        incoming_id *= 10;
        if(transmissionArray[i]-'0' < 0 || transmissionArray[i]-'0'>9)
            return;
        incoming_id += transmissionArray[i]-'0';//(transmissionArray[i]-'0')*(pow(10, i));
    }
    Serial.print("product id: "); Serial.println(incoming_id);
    
    if(transmissionArray[5] == ':') {
        PDC_sync = true;
        return;
    }
    
//    if(transmissionArray[5] != ',') {
////        Serial.println("Error: invalid product id");
//    }
//    
//    for(chk_indx=6;transmissionArray[chk_indx] != ',';chk_indx++) {
//        transmission_id *= 10;
//        transmission_id += transmissionArray[chk_indx]-'0';
//    }
//    Serial.println(transmission_id);
//
//    for(++chk_indx;transmissionArray[chk_indx] != ';';chk_indx++) {
//        checksum *= 10;
//        checksum += transmissionArray[chk_indx]-'0';//(transmissionArray[chk_indx]-'0')*(pow(10, chk_indx-initial));
//    }
//    Serial.print("check sum "); Serial.println(checksum);
//    
//    for(++chk_indx; transmissionArray[chk_indx] != ':'; chk_indx++) {
//        char_to_int = transmissionArray[chk_indx]-'0';
//        if(char_to_int == -1)
//            Serial.println("Error"); //: invalid IR code");
//        else if(char_to_int >= 0 && char_to_int < 10 )
//            num_chars++;
//            
//    }
//    Serial.print("num_chars: "); Serial.println(num_chars);
////    if(num_chars != checksum)
////        Serial.println("Error: wrong number of codes received");
//
//    
}

// getChar
char PDCreceive::getChar(int index) {
    return transmissionArray[index];
}

// parseTransmission takes a completed transmission character by character and
// parses it into useful information to tell the PDC what to do. returns false if
// there is an error

bool PDCreceive::parseTransmission(unsigned int write_array[][10]) {
    int chk_indx;
    bool error_found = false;
    long incoming_id = 0;
    int h_index = 0;
    int v_index = 0;

    //should have already been checked by checkTransmission();
//    // product id is always 5 numbers
//    for(int i=0; i<5; i++) {
//        incoming_id *= 10;
//        incoming_id += transmissionArray[i]-'0';//(transmissionArray[i]-'0')*(pow(10, i));
//    }
////    Serial.print("product id: "); Serial.println(incoming_id);
//    
//    if(transmissionArray[5] == ':') {
//        PDC_sync = true;
//        return true;
//    } else if(transmissionArray[5] != ',') {
//        error_found = true;
//    }
    
    // transmission id is always 3 digits
    for(chk_indx=6;chk_indx<9;chk_indx++) {
        transmission_id *= 10;
        transmission_id += transmissionArray[chk_indx]-'0';
//        Serial.print("transmission ID: "); Serial.println(transmission_id);
    }
    Serial.print("transmission ID: "); Serial.println(transmission_id);
    
    if(transmissionArray[9] == ':') {
        command_query = true;
        return true;
    } else if(transmissionArray[9] != ',') {
        command_write = true;
    }
    
    chk_indx = checkCharSum(chk_indx, &error_found);
    
    if(error_found) {
        Serial.println("error found in parseTransmission");
        return false;
    }
    
    // if the transmission is good and the command is a write_command, store the
    // remaining values into write_array
    for(chk_indx; transmissionArray[chk_indx] != ':'; chk_indx++) {
        if(transmissionArray[chk_indx] == ';') {
//            Serial.print("write array "); Serial.print(h_index*10+v_index);
//            Serial.print(": "); Serial.println(write_array[v_index][h_index]);

            h_index++;
            v_index = 0;
        } else if(transmissionArray[chk_indx] == ',') {
            v_index++;
        } else {
            write_array[v_index][h_index] = write_array[v_index][h_index]*10 + transmissionArray[chk_indx] - '0';
//            Serial.print("write array "); Serial.print(h_index*10+v_index);
//            Serial.print(": "); Serial.println(write_array[v_index][h_index]);
        }
    }
    return true;
}

// check if the transmission array has all legitimate characters and if the checksum
// is correct

int PDCreceive::checkCharSum(int chk_indx, bool *error_found) {
    int checksum = 0;
    int num_chars = 0;
    short char_to_int;
    int data_index;
    
    for(++chk_indx;transmissionArray[chk_indx] != ';';chk_indx++) {
        checksum *= 10;
        checksum += transmissionArray[chk_indx]-'0';//(transmissionArray[chk_indx]-'0')*(pow(10, chk_indx-initial));
    }
    //    Serial.print("check sum "); Serial.println(checksum);
    data_index = chk_indx+1;
    
    for(++chk_indx; transmissionArray[chk_indx] != ':'; chk_indx++) {
        char_to_int = transmissionArray[chk_indx]-'0';
        if(char_to_int == -1) {
            *error_found = true;
            //            Serial.println("Error"); //: invalid IR code");
        } else if(char_to_int >= 0 && char_to_int < 10 ) {
            num_chars++;
        }
    }
    Serial.print("num_chars: "); Serial.println(num_chars);
    if(num_chars != checksum)
        *error_found = true;
    return data_index;
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
    memset(transmissionArray, 0, sizeof(transmissionArray));
    PDC_in_transmission = false;
    transmission_complete = false;
    PDC_sync = false;
    command_query = false;
    command_write = false;
    transmission_id = 0;
}


long PDCreceive::pow(int base, int power) {
    long ans = 1;
    for(int i=0;i<power;i++) {
        ans *= base;
    }
    return ans;
}
