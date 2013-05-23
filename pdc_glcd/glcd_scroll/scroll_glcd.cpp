#include "scroll_glcd.h"

scroll_glcd:: scroll_glcd(){
  int old_pointy;
  int old_pointx;
}


void scroll_glcd:: Hscroll(int maxY, int minX, int maxX, int Analogpin){
  int ay = maxY - 5;
  int ax = minX + 3;
  int by = ay;
  int bx = maxX - 3;
  
  left (ax, ay);
  right (bx, by);
  int sc_max = maxX - 6;
  int sc_min = minX + 6;
  
  //Scrolling through analogRead on pin AnalogPin
  
  int sc_point = map(analogRead(Analogpin), 0, 1023, sc_min, sc_max);
  
  //Scrolling location, based on AnalogPin
  if (sc_point != old_pointx)
    {
      h_stopw(old_pointx, by);
    }
  old_pointx = sc_point;
  h_stop(sc_point, by);
  
}

void scroll_glcd::Vscroll(int maxX, int minY, int maxY, int Analogpin){
  int ax = maxX - 5;
  int ay = minY + 3;
  int bx = ax;
  int by = maxY - 3;
  
  up (ax, ay);    
  down (bx, by);    //bx = ax
  int sc_max = maxY - 6;
  int sc_min = minY + 6;
  
  //Scrolling through analogRead on pin AnalogPin
  
  int sc_point = map(analogRead(Analogpin), 0, 1023, sc_min, sc_max);
  
  //Scrolling location, based on AnalogPin
  if (sc_point != old_pointy)
    {
      v_stopw(bx, old_pointy);
    }
  old_pointy = sc_point;
  v_stop(bx, sc_point);
  
}

//----------------------------------------------------------------------------------------------
void scroll_glcd::up(int x, int y){
for(int i = x; i <= x + 4; i++){
    GLCD.SetDot(i, y, BLACK);
  }
  for(int i = x + 1; i <= x + 3; i++){
    GLCD.SetDot(i,y - 1, BLACK);
  }
  for(int i = x+2; i <= x + 2; i++){
    GLCD.SetDot(i, y - 2, BLACK);
  }
}

void scroll_glcd::down(int x, int y){
  for(int i = x; i <= x + 4; i++){
    GLCD.SetDot(i, y - 2, BLACK);
  }
  for(int i = x + 1; i <= x + 3; i++){
    GLCD.SetDot(i,y - 1, BLACK);
  }
  for(int i = x+2; i <= x + 2; i++){
    GLCD.SetDot(i, y, BLACK);
  }
}

//----------------------------------------------------------------------------------------------

void scroll_glcd::v_stop(int x, int y){
for(int i = x; i <= x + 4; i++){
  int j = y;
  for (j ; j >= y - 2; j--){
      GLCD.SetDot(i, j, BLACK);
    }
  }
}

void scroll_glcd::v_stopw(int x, int y){
for(int i = x; i <= x + 4; i++){
  int j = y;
  for (j ; j >= y - 2; j--){
      GLCD.SetDot(i, j, WHITE);
    }
  }
}

//----------------------------------------------------------------------------------------------

void scroll_glcd::right(int x, int y)
{
  for(int i = y; i >= y - 4; i--){
      GLCD.SetDot(x, i, BLACK);
    }
    for(int i = y - 1; i >= y - 3; i--){
      GLCD.SetDot(x + 1, i, BLACK);
    }
    for(int i = y-2; i >= y - 2; i--){
      GLCD.SetDot(x + 2, i, BLACK);
    }
}

void scroll_glcd::left(int x, int y)
{
  for(int i = y - 2; i >= y - 2; i--){
      GLCD.SetDot(x, i, BLACK);
    }
    for(int i = y - 1; i >= y - 3; i--){
      GLCD.SetDot(x + 1, i, BLACK);
    }
    for(int i = y; i >= y - 4; i--){
      GLCD.SetDot(x + 2, i, BLACK);
    }
}

//----------------------------------------------------------------------------------------------

void scroll_glcd::h_stop(int x, int y){
  int j = x;
    for (j ; j <= x + 2; j++){
      for(int i = y; i >= y - 4; i--){
      GLCD.SetDot(j, i, BLACK);
    }
  }
}

void scroll_glcd::h_stopw(int x, int y){
  int j = x;
    for (j ; j <= x + 2; j++){
      for(int i = y; i >= y - 4; i--){
      GLCD.SetDot(j, i, WHITE);
    }
  }
}

//----------------------------------------------------------------------------------------------
