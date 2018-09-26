// ADCTestMain.c
// Runs on TM4C123
// This program periodically samples ADC channel 0 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// September 5, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// center of X-ohm potentiometer connected to PE3/AIN0
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V 

/*
#include <stdint.h>
#include "ST7735.h"
#include <limits.h>
#include "ADCSWTrigger.h"
#include "tm4c123gh6pm.h"
#include "PLL.h"

#define PF4  	 				  (*((volatile uint32_t *)0x40025040))
#define PF2             (*((volatile uint32_t *)0x40025010))
#define PF1             (*((volatile uint32_t *)0x40025008))
	
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void (*PeriodicTask)(void);
void Timer0A_Init100HzInt(void);
void Timer1A_Init100HzInt(void);
void Graph_PMF(void);
int getTimeJitter(void);
void DelayWait10ms(uint32_t n);
void Pause(void);
void testAverager(void);

int counter = 0;
int timeJitter;
volatile uint32_t ADCvalue;
int array_time[1000];
uint32_t array_ADCvalue[1000];
static volatile uint32_t distribution[4096];


	int main(void){
  PLL_Init(Bus80MHz);                   // 80 MHz
	SYSCTL_RCGCSSI_R |= 0x01;
  SYSCTL_RCGCGPIO_R |= 0x20;            // activate port F
	ST7735_InitR(INITR_REDTAB);
  ADC0_InitSWTriggerSeq3_Ch9();         // allow time to finish activating
	while((SYSCTL_PRGPIO_R&0x20)==0){}; // allow time for clock to start
  Timer0A_Init100HzInt();
	Timer1A_Init100HzInt();			// set up Timer0A for 100 Hz interrupts
	//ADC0_SAC_R = ADC_SAC_AVG_64X;	
		
	GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00F)+0x00000000;
  GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x06;          // disable alt funct on PF2, PF1
  GPIO_PORTF_DEN_R |= 0x06;             // enable digital I/O on PF2, PF1
                                        // configure PF2 as GPIO
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
	//added for switch 1
	GPIO_PORTF_PUR_R |= 0x10;         // 5) pullup for PF4
	////
  PF2 = 0;                      // turn off LED
  EnableInterrupts();
	testAverager();
//	ADC0_SAC_R = ADC_SAC_AVG_4X;	
//	testAverager();
//	ADC0_SAC_R = ADC_SAC_AVG_16X;	
//	testAverager();
//	ADC0_SAC_R = ADC_SAC_AVG_64X;
	
}
void testAverager(){
	while(1){
		while(counter < 1000){
			PF1 ^= 0x02;  // toggles when running in main
			//GPIO_PORTF_DATA_R ^= 0x02; // toggles when running in main
			PF1 = (PF1*12345678)/1234567+0x02; // this line causes jitter
		}

				// after filling the arrays, find the jitter
				int jitter = getTimeJitter();
				
				// find the minimum and maximum data, then find the range
				ST7735_FillScreen(ST7735_BLACK);
				ST7735_SetCursor(0,1);
				ST7735_OutString("Time Jitter: ");
				ST7735_OutUDec(jitter);
				Graph_PMF();
		}
	}
*/
// This debug function initializes Timer0A to request interrupts
// at a 100 Hz frequency.  It is similar to FreqMeasure.c.
/*
void Timer0A_Init100HzInt(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = 799999;         // start value for 100 Hz interrupts
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}

void Timer1A_Init100HzInt(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x02;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER1_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER1_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER1_TAILR_R = 799999;         // start value for 100 Hz interrupts
  TIMER1_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER1_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}


void Timer1A_Init(void(*task)(void), uint32_t period){
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  PeriodicTask = task;          // user function
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = 0xFFFFFFFF;    // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  //TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  //NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00008000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 37, interrupt number 21
  NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}

void Timer2A_Init(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x04;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER2_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER2_TAILR_R = 7989;         // start value for close to 10 kHz interrupts
  TIMER2_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER2_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x20000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout
  PF2 ^= 0x04;                   // profile
  PF2 ^= 0x04;                   // profile
  ADCvalue = ADC0_InSeq3();
  PF2 ^= 0x04;                   // profile
	//debugging dumps
	if (counter < 1000){
		array_time[counter] = TIMER1_TAR_R;
		array_ADCvalue[counter] = ADCvalue;
		counter ++;
	}
} */

/*
void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
  (*PeriodicTask)();                // execute user task
}

void Timer2A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout
  PF2 ^= 0x04;                   // profile
  PF2 ^= 0x04;                   // profile
  ADCvalue = ADC0_InSeq3();
  PF2 ^= 0x04;                   // profile
	if (counter < 1000) {
		array_time[counter] = TIMER1_TAR_R;
	  array_ADCvalue[counter] = ADCvalue;		
		counter++;
	}
}

int getTimeJitter(void){
		int timeDiff[999];		//new array for time difference
	  int largest = 0;
		int smallest = array_time[0] - array_time[1];
		for (int i = 0; i < 999; i++) {
			timeDiff[i] = array_time[i] - array_time[i+1];
			if (largest < timeDiff[i]){
				largest = timeDiff[i];
			}
			if (smallest > timeDiff[i]){
				smallest = timeDiff[i];
			}
		}
		return (largest - smallest);
}	

void Graph_PMF(){
		uint32_t pmf[128];
		int min_X;
		uint32_t max_Y, max_Y_index = 0;	//peak occurance, and value of the peak
		for (int i=0;i<=999;i++){
			distribution[array_ADCvalue[i]]++;
		}
		for (int i=0;i<=4095;i++)					//find the peak occurance, and it's value/index in the distribution array
			if (distribution[i] > max_Y) {
				max_Y = distribution[i];
				max_Y_index = i;
		}
		min_X = max_Y_index - 64; //start from left half
		ST7735_SetCursor(0,0);
		ST7735_OutString("ADC: ");
		ST7735_OutUDec(max_Y_index);
		if(min_X < 0){
			min_X = 0;
		}
		else if ((min_X + 127) > 4095){
			min_X = 3968;
		}
		for (int i=0;i<=127;i++){
			pmf[i] = distribution[min_X + i] * 128/1000;
		}
		
		// Draw pmf
		ST7735_DrawFastHLine(0, 159, 127, ST7735_WHITE);
		for (int i=0;i<=127;i++){
			ST7735_DrawFastVLine(i,32+(127-pmf[i]),pmf[i],ST7735_WHITE);
			ST7735_SetCursor(0,3);
		}
		counter = 0;  
		ST7735_SetCursor(0,2);
		ST7735_OutString("Averager: ");
		ST7735_OutUDec(ADC0_SAC_R);
}

// Subroutine to wait 10 msec
// Inputs: None
// Outputs: None
void DelayWait10ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}

// click PF4 button to continue
void Pause(void){
  while(PF4==0x00){ 
    DelayWait10ms(10);
  }
  while(PF4==0x10){
    DelayWait10ms(10);
  }
}

*/
