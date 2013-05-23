#include<Arduino.h>  
  struct timer {
  int time_val;
  int lastTime;
  int lastSection;
  int storedTime;
  int last_section_time; 
  int count; 
  unsigned long sectionTime[15];
  int steps[50];
  timer(){
    time_val=0;
    lastTime=0;
    lastSection=0;
    storedTime=0;
    count=0; 
    last_section_time=0;
    for (int i=0; i<15; i++)
      {
        sectionTime[i]=0;
      }
     for (int i=0; i<50; i++){
      steps[i]=-1;
    }
  }
  } ;
 
 
 
 
