#include "button.h"

button:: button(){
  lastDebounceTime=0;
  debounce_delay=600000; 
  pressed=false; 
  interrupt_pin=0; 
}

button:: button(int pin){
  lastDebounceTime=0;
  debounce_delay=600000; 
  pressed=false; 
  interrupt_pin=pin ; 
}
void button::rise(){
  long currentTime = micros();
  if ((currentTime - lastDebounceTime) > debounce_delay)
  {
    lastDebounceTime = currentTime;
    pressed=!pressed; 
  }
}

