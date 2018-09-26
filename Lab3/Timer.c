// Timer.c

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "SysTick.h"
#include "ST7735.h"
#include "Timer.h"

int time = 8*3600 +46*60 - 25;
char sec[]={0,0,'\0'};
char min[]={0,0,'\0'};
char hour[]={0,0,'\0'};
uint32_t h,m,s; // time as numbers

int updateTime(int flag, int time) {
   if (flag) 
		 return ((time+1)%(3600*12));
   else 
		 return time;
}

/* Display the digital clock*/
void outputTime(int time, uint8_t cursor_num){
   
   int hour = (time / 3600) % 12;
   int minute = (time / 60) % 60;
   int second = (time % 60);
   
   ST7735_SetCursor(6,cursor_num);
   
	 if(hour == 0) {
		  ST7735_OutUDec(12);
	 }
   else if( hour < 10){
      ST7735_OutUDec(0);
		  ST7735_OutUDec(hour);

   } else {
		    ST7735_OutUDec(hour);
	 }
   
   ST7735_OutChar(':');
   if(minute < 10){
      ST7735_OutUDec(0);
   }
   ST7735_OutUDec(minute);
   
   ST7735_OutChar(':');
   if(second < 10){
      ST7735_OutUDec(0);
   }
   ST7735_OutUDec(second);
}

/* return a string of two digits */
void getSeconds(int time, char secStr[2]) {
   s = time%60;
   if (s < 10) {
      secStr[0] = '0';
      secStr[1] = 48 + s;
   } else {
      secStr[0] = 48 + s/10;
      secStr[1] = 48 + s%10;
   }

}

void getMinutes(int time, char minStr[2]) {
   m = (time/60)%60;
   if (m < 10) {
      minStr[0] = '0';
      minStr[1] = 48 + m;
   } else {
      minStr[0] = 48 + m/10;
      minStr[1] = 48 + m%10;
   }
}
void getHours(int time, char hourStr[2]) {
   h = (time/3600)%12;
	 if (h == 0) {
		 hourStr = "12";
	 }
   else if (h < 10) {
      hourStr[0] = '0';
      hourStr[1] = 48 + h;
   } else {
      hourStr[0] = 48 + h/10;
      hourStr[1] = 48 + h%10;
   }   
}
