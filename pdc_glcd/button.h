#include <arduino.h>

class button {
  public:
    button(); 
    button(int); 
    void rise();
    int interrupt_pin;
    bool pressed; 
  private:
    long debounce_delay;
    volatile int buttonState; 
    volatile long lastDebounceTime; 
    volatile bool switchState; 
};


