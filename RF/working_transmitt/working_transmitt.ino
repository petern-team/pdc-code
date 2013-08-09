// Transmitt

#include <VirtualWire.h>

const int led_pin = 13;
const int transmit_pin = 9;
//const int receive_pin = 2;
const int transmit_en_pin = 3;
const int pwmPin = 5;
void setup()
{
  Serial.begin(9600);
    // TCCR0B = xxxxx001, frequency is 64kH
    // TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
    // TCCR0B = _BV(CS00);

    pinMode(pwmPin, OUTPUT);    

    // Initialise the IO and ISR
    vw_set_tx_pin(transmit_pin);
    //  vw_set_rx_pin(receive_pin);
    vw_set_ptt_pin(transmit_en_pin);
    //vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2400);	 // Bits per sec
    
    analogWrite(pwmPin, 128);
}

//byte count = 1;

void loop()
{
    //char msg[7] = {'h','e','l','l','o',' ','#'};
    char msg[27] = {'a','b','c','d','e',
            'f','g','h','i','j','k','l',
            'm','n','o','p','q','r','s',
            't','u','v','w','x','y','z',
            '0'};


    //msg[6] = count;
//    digitalWrite(led_pin, HIGH); // Flash a light to show transmitting
    Serial.println("sending...");
    vw_send((uint8_t *)msg, 27);
    vw_wait_tx(); // Wait until the whole message is gone
//    digitalWrite(led_pin, LOW);
    delay(1000);
    //count = count + 1;
    
}


