#include <glcd.h>
#include <fonts/allFonts.h>
#include "arrow.h"
#include "scroll_glcd.h"

int scrollint = A5;

scroll_glcd glcd;

void setup() {

  GLCD.Init();
  GLCD.SelectFont(System5x7);
  pinMode(scrollint, INPUT);
  
  //IMPORTANT: delete up and down if they work from the library
  up(123,2);
  down(123, 63);
}

void loop() {

  glcd.Vscroll( 20, 5, 60, A5);
  glcd.Hscroll(25, 30, 100, A5);
          //left(x,y); //DONE
          //right(x,y); //DONE
          //up(x,y); //DONE
          //down(x,y); //DONE
          //v_stop(x,y); //DONE
          //h_stop(x,y); //DONE
}

