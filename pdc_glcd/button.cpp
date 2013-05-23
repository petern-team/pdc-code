#include "button.h"

button:: button(){
  lastDebounceTime=0;
  debounce_delay=200; //time in ms to delay further button activation
  pressed=false; 
  interrupt_pin=0; //the pin the button is on
}

button:: button(int pin){
  lastDebounceTime=0;
  debounce_delay=200; // time in ms to delay further button activations
  pressed=false; 
  interrupt_pin=pin ; //the pin the button is on
}
void button::rise(){
  long currentTime = millis();
  if ((currentTime - lastDebounceTime) > debounce_delay)
  {
    lastDebounceTime = currentTime;
    pressed=!pressed; 
  }
}
//use a function like this as the interrupt service routine in your main code
//add a line like "attachInterrupt(button_2.interrupt_pin, rise2_funct, RISING)"
/*void rise2_funct(){//the interrupt service routine. Calls the rise function in the button2 class. Handles the graph button
  noInterrupts();
  button_2.rise();
  interrupts();
}*/

