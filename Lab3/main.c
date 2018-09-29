/* main.c
*/

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "SysTick.h"
#include "ST7735.h"
#include "Timer.h"
#include "LCD.h"
#include "Speaker.h"
#include "Button.h"
#include "main.h"
#include "ADCSWTrigger.h"

#define PF3                     (*((volatile uint32_t *)0x40025020))
#define PF2                     (*((volatile uint32_t *)0x40025010))
#define PF1                     (*((volatile uint32_t *)0x40025008))
//#define PD0                     (*((volatile uint32_t *)0x40007

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode


extern uint32_t h,m,s; // time as numbers
extern char hour[],min[],sec[], sec_s[], min_s[];
extern int time, time_sw, time_d;
extern int alarm, inAlarm;
extern int secFlag, sw_flag;
int lastTimePressed, timeInactive = 0;
uint32_t ADCvalue;


	
phase phases[7] = {
   {{"\n"}, {'\n'}, -1, {'\n'}, 0},  													// phase 0: clock display
   {{"Set Clock", "Set Alarm", "Back", "Stop Watch"}, {'\n'}, 0, {ST7735_YELLOW,ST7735_WHITE,ST7735_WHITE, ST7735_WHITE}, 0},   // phase 1: select menu
   {{"Set", "Back"}, {0, 0, 0}, 2, {ST7735_WHITE,ST7735_WHITE,ST7735_YELLOW,ST7735_WHITE,ST7735_WHITE}, 0},               // phase 2: set time
   {{"Set", "Back"}, {0, 0, 0}, 2, {ST7735_WHITE,ST7735_WHITE,ST7735_YELLOW,ST7735_WHITE,ST7735_WHITE}, 0},	              // phase 3: set alarm
	 {{"Start", "Pause", "Back"},{'\n'}, 0, {ST7735_YELLOW,ST7735_WHITE,ST7735_WHITE}, 0}, 	// phase 4: stop watch display
	 {{"\n"}, {'\n'}, -1, {'\n'}, 0},  													// phase 5: clock display 2
	 {{"\n"}, {'\n'}, -1, {'\n'}, 0},  													// phase 6: clock display 3
};
uint8_t phase_num = 0;

//initiate speaker
void PortD_Init(void){
   SYSCTL_RCGCGPIO_R |= SYSCTL_RCGC2_GPIOD;        // 1) activate port D
   uint8_t delay = SYSCTL_RCGC2_R; ;   // allow time for clock to stabilize
   // 2) no need to unlock PD3-0
   GPIO_PORTD_AMSEL_R &= ~0x0F;      // 3) disable analog functionality on PD3-0
   GPIO_PORTD_PCTL_R &= ~0x0000FFFF; // 4) GPIO
   GPIO_PORTD_DIR_R |= 0x0F;         // 5) make PD0 out
   GPIO_PORTD_AFSEL_R &= ~0x0F;      // 6) regular port function
   GPIO_PORTD_DEN_R |= 0x0F;         // 7) enable digital I/O on PD0
}



int main(){
   
   SYSCTL_RCGCGPIO_R |= 0x20;  // activate port F
   PLL_Init(Bus80MHz);                   // 80 MHz
   PortF_Init();
   PortD_Init();
	 SysTick_Init();
   ADC0_InitSWTriggerSeq3_Ch9();
   ST7735_InitR(INITR_REDTAB);
   EdgeInterrupt_Init();
   EnableInterrupts();
   

	 // Initial Display
   ST7735_FillScreen(ST7735_BLACK);
	 outputTime(time, 2);	
   drawFace();
   drawHands(time);
	 lastTimePressed = time;
	 timeInactive = 0;
   
   while(1){
	    long sr = StartCritical();
      int tempTime = time;
      //GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R^0x04; // toggle PF2
      
      time = updateTime(secFlag, time);
      alarm = checkAlarm(time);
      if(time != tempTime){ // if time changed, redraw, reset flag, check alarm
         secFlag = 0;
      }
			timeInactive = time - lastTimePressed;
			
      switch (phase_num) {
         case 0:
         if(time != tempTime){ // if time changed, redraw, reset flag, check alarm
            outputTime(time, 2);	
         }
         if((time % 60) == 0){ // every minute, erase hand and draw again
            eraseHands(time - 60);
            drawHands(time);
         }
         break;
         case 1:
				 if(timeInactive >= 25){
						phase_num = 0; 
						 ST7735_FillScreen(ST7735_BLACK);   // clear the screen
						 drawFace();
						 drawHands(time);
					   outputTime(time, 2);
					  break;
				   }
         ST7735_DrawString(6,4,phases[1].options[0], phases[1].color[0]);
         ST7735_DrawString(6,6,phases[1].options[1], phases[1].color[1]);
         ST7735_DrawString(6,8,phases[1].options[2], phases[1].color[2]);
				 ST7735_DrawString(6,10,phases[1].options[3], phases[1].color[3]);
         
         break;
         case 2:
				 if(timeInactive >= 25){					//go back to the initial display after the 25sec inactivity
						phase_num = 0; 
						ST7735_FillScreen(ST7735_BLACK);   // clear the screen
						drawFace();
						drawHands(time);
					  outputTime(time, 2);
					  break;
				   }
         ADCvalue = ADC0_InSeq3();
         if (phases[2].selected) {
            if (phases[2].highlight == 2) { // hour: 1-12
               h = ADCvalue*12/4096 + 1; // use 4096 to avoid the hour turns to 13
               if (h < 10) {
                  hour[0] = '0';
                  hour[1] = 48 + h;
               } else {
                  hour[0] = 48 + h/10;
                  hour[1] = 48 + h%10;
               }   
            }
            else if (phases[2].highlight == 3) { // min: 0-59
               m = ADCvalue*60/4096; // 4096 to avoid 60
               if (m < 10) {
                  min[0] = '0';
                  min[1] = 48 + m;
               } else {
                  min[0] = 48 + m/10;
                  min[1] = 48 + m%10;
               }   
            }
            else if (phases[2].highlight == 4) { // sec: 0-59
               s = ADCvalue*60/4096;
               
               if (s < 10) {
                  sec[0] = '0';
                  sec[1] = 48 + s;
               } else {
                  sec[0] = 48 + s/10;
                  sec[1] = 48 + s%10;
               }   
            }
         }
         ST7735_DrawString(8,8,phases[2].options[0], phases[2].color[0]);
         ST7735_DrawString(8,10,phases[2].options[1], phases[2].color[1]);
         ST7735_SetCursor(6, 6);
         
         ST7735_DrawString(6,6,hour,phases[2].color[2]);
         ST7735_DrawString(8,6,":",ST7735_WHITE);
         ST7735_DrawString(9,6,min,phases[2].color[3]);
         ST7735_DrawString(11,6,":",ST7735_WHITE);
         ST7735_DrawString(12,6,sec,phases[2].color[4]);
         break;
				 
         case 3:
					 if(timeInactive >= 25){
						phase_num = 0; 
						 ST7735_FillScreen(ST7735_BLACK);   // clear the screen
						 drawFace();
						 drawHands(time);
						 outputTime(time, 2);
					  break;
				   }
					 ADCvalue = ADC0_InSeq3();
					 if (phases[3].selected) {
            if (phases[3].highlight == 2) { // hour: 1-12
               h = ADCvalue*12/4096 + 1; // use 4096 here to avoid 13
               if (h < 10) {
                  hour[0] = '0';
                  hour[1] = 48 + h;
               } else {
                  hour[0] = 48 + h/10;
                  hour[1] = 48 + h%10;
               }   
            }
            else if (phases[3].highlight == 3) { // min: 0-59
               m = ADCvalue*60/4096; // use 4096 to avoid 60
               if (m < 10) {
                  min[0] = '0';
                  min[1] = 48 + m;
               } else {
                  min[0] = 48 + m/10;
                  min[1] = 48 + m%10;
               }   
            }
            else if (phases[3].highlight == 4) { // sec: 0-59
               s = ADCvalue*60/4096;
               
               if (s < 10) {
                  sec[0] = '0';
                  sec[1] = 48 + s;
               } else {
                  sec[0] = 48 + s/10;
                  sec[1] = 48 + s%10;
               }   
            }
         }
         ST7735_DrawString(8,8,phases[3].options[0], phases[3].color[0]);
         ST7735_DrawString(8,10,phases[3].options[1], phases[3].color[1]);
         ST7735_SetCursor(6, 6);
         ST7735_DrawString(6,6,hour,phases[3].color[2]);
         ST7735_DrawString(8,6,":",ST7735_WHITE);
         ST7735_DrawString(9,6,min,phases[3].color[3]);
         ST7735_DrawString(11,6,":",ST7735_WHITE);
         ST7735_DrawString(12,6,sec,phases[3].color[4]);
         break;
				 
				 case 4:
					 if(timeInactive >= 100){							//100s for stopwatch
						 phase_num = 0; 
						 ST7735_FillScreen(ST7735_BLACK);   // clear the screen
						 drawFace();
						 drawHands(time);
						 outputTime(time, 2);
					  break;
				   }
					 if(time != tempTime && sw_flag == 1){ // if time changed, redraw, reset flag, check alarm
						time_sw = time - time_d;
            outputTimer(time_sw, 6);	
           }
				 ST7735_SetTextColor(ST7735_WHITE);
				 outputTimer(time_sw, 6);
				 ST7735_DrawString(8,8,phases[4].options[0], phases[4].color[0]);
         ST7735_DrawString(8,10,phases[4].options[1], phases[4].color[1]);
				 ST7735_DrawString(8,12,phases[4].options[2], phases[4].color[2]);
				 break;
					
				 case 6:
						outputTime(time, 7);
						if(time != tempTime){ // if time changed, redraw, reset flag, check alarm
							outputTime(time, 7);	
						}
         break;
				 
				 case 5:
					if(time != tempTime){
						drawFace();
						drawHands(time);
						if((time % 60) == 0){ // erase hand and draw again every minute
							drawFace();
							eraseHands(time-60);
							drawHands(time);
						}
					}
					break;
      }
			
      EndCritical(sr);  
   }
}
