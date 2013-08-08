
// receive

#include <VirtualWire.h>

const int led_pin = 2;
//const int transmit_pin = 12;
const int receive_pin = 11;
const int transmit_en_pin = 4;

void setup()
{
    delay(1000);
    Serial.begin(9600);	// Debugging only
    Serial.println("setup");
//    pinMode(led_pin, OUTPUT);

    // Initialise the IO and ISR
    //vw_set_tx_pin(transmit_pin);
    vw_set_rx_pin(receive_pin);
    vw_set_ptt_pin(transmit_en_pin);
    //vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2400);	 // Bits per sec

    vw_rx_start();       // Start the receiver PLL running
}

void loop()
{
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;
    // Serial.println(VW_MAX_MESSAGE_LEN);
    
    //uint8_t buf[5];
    //uint8_t buflen = 5;

    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
//        digitalWrite(led_pin, HIGH); // Flash a light to show received good message
        // Message with a good checksum received, print it.
        Serial.print("\n Got: ");

        for (int i = 0; i < buflen; i++) {
            //Serial.print(buf[i], HEX);
//            the_char = buf[i];
            Serial.write(buf[i]);
            //Serial.print(buf[i]);
        }
        //Serial.print("\n");
//        delay(1000);
//        digitalWrite(led_pin, LOW);
//        delay(1000);
    }
    /*
    else
    {
        Serial.println("No Data");
        delay(1000);
    }
    */
}
