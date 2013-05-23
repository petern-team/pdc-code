/*
  The glcd has (128 x 64)px 
    >> x axis is left-to-right (0 - 127)
    >> y axis is top-to-bottom (0 - 63)
  The (x,y) coordinates used in these functions are for the lower left corner
    >> for the down arrow * * * * *
                            * * *
                          #   *
            # represents the pixel represented in the lower left corner
*/
int oldy = 200;

void right(int x, int y)
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

void left(int x, int y)
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

void h_stop(int x, int y){
  int j = x;
    for (j ; j <= x + 2; j++){
      for(int i = y; i >= y - 4; i--){
      GLCD.SetDot(j, i, BLACK);
    }
  }
}

//UP AND DOWN ARROWS

void up(int x, int y){
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

void down(int x, int y){
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

void v_stop(int x, int y){
for(int i = x; i <= x + 4; i++){
  int j = y;
  for (j ; j >= y - 2; j--){
      GLCD.SetDot(i, j, BLACK);
    }
  }
}

void v_stopw(int x, int y){
for(int i = x; i <= x + 4; i++){
  int j = y;
  for (j ; j >= y - 2; j--){
      GLCD.SetDot(i, j, WHITE);
    }
  }
}
