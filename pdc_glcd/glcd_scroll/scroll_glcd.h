#include <arduino.h>
#include <glcd.h>
#include <fonts/allFonts.h>


class scroll_glcd {
  public:
    scroll_glcd(); 
    void Hscroll(int maxY, int minX, int maxX, int Analogpin);
    void Vscroll(int maxX, int minY, int maxY, int Analogpin);
    
    void down(int x, int y);
    void up(int x, int y);
    
    void left(int x, int y);
    void right(int x, int y);
    
    void v_stop(int x, int y);
    void h_stop(int x, int y);
    
    void v_stopw(int x, int y);
    void h_stopw(int x, int y);
    
    
    
    
    //int interrupt_pin;
    //bool pressed; 
  private:
  // can call map from here
  int old_pointy;
  int old_pointx;
};
