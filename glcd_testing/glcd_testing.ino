/*  Example 24.4 - KS0108 Graphic LCD demonstration
http://tronixstuff.com/tutorials > chapter 24  CC by-sa-nc
*/
#include <ks0108.h>  // library header
int xc, yc = 0;
int d=1000; // for delay use
void setup()
{
  Serial.begin(9600);
  Serial.println("setup");
GLCD.Init(NON_INVERTED);   // initialise the library with pixel default as off
GLCD.ClearScreen();        // clear the LCD
randomSeed(analogRead(5));
Serial.println("leaving setup");
}
void loop()
{
GLCD.DrawRect(0, 0, 127, 63, BLACK);
// draw an open rectangle that spans the extremties of the LCD
GLCD.DrawRect(10, 10, 117, 53, BLACK);
GLCD.DrawRect(20, 20, 107, 43, BLACK);
GLCD.DrawRect(30, 30, 97, 33, BLACK);
delay(d);
GLCD.ClearScreen();  // clear the LCD
for (int a=1; a<20; a++)
{
GLCD.DrawCircle(63,31,a,BLACK);
// draws a circle with centre at 61,31; radius of a, with black pixels
delay(d-800);
GLCD.DrawCircle(63,31,a,WHITE); // draws the same circle with the pixels off
}
delay(d);
GLCD.ClearScreen();  // clear the LCD
for (int a=0; a<128; a++)
{
GLCD.DrawVertLine(a, 0, 63, BLACK);
// draws a vertical line from xy position a, 0 of length 63
delay(d-950);
}
delay(d-800);
for (int a=0; a<128; a++)
{
GLCD.DrawVertLine(a, 0, 63, WHITE);
delay(d-950);
}
GLCD.ClearScreen();  // clear the LCD
for (int a=0; a<64; a++)
{
GLCD.DrawHoriLine(0, a, 127, BLACK);
// draws a horizontal line from xy position 0, a of length 127
delay(d-950);
}
for (int a=0; a<64; a++)
{
GLCD.DrawHoriLine(0, a, 127, WHITE);
delay(d-950);
}
GLCD.ClearScreen();  // clear the LCD
GLCD.DrawRoundRect(30, 30, 20,20, 5,BLACK);
// draw a rectangle with rounded edges: x, y, width, height, radius of rounded edge, colour
GLCD.DrawRoundRect(60, 30, 20,20, 5,BLACK);
delay(d);
GLCD.ClearScreen();  // clear the LCD
delay(d);
GLCD.FillRect(30, 30, 30, 10, BLACK);
// draws a filled rectangle: x, y, width, height, colour
delay(d);
GLCD.ClearScreen();  // clear the LCD
for (int a=0; a<1000; a++)
{
xc=random(0,127);
yc=random(0, 63);
GLCD.SetDot(xc, yc, BLACK);
// turn on a pixel at xc, yc);
delay(2);
}
GLCD.ClearScreen();
// clear the LCD
}
