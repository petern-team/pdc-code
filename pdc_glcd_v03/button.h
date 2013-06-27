#include <arduino.h>

class button {
  public:
    button(); 
    button(int); 
    void rise();
    int interrupt_pin;
    bool pressed; 
  private:
    static const long debounce_delay = 200;
    volatile int buttonState; 
    volatile long lastDebounceTime; 
    volatile bool switchState; 
};


