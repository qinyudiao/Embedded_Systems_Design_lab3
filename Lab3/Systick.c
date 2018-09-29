// SysTick.c
// Runs on LM4F120/TM4C123

#include <stdint.h>
#include "tm4c123gh6pm.h"
#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_M        0x00FFFFFF  // Counter load value
#define PF2                     (*((volatile uint32_t *)0x40025010))

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
   NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
   NVIC_ST_RELOAD_R = 799999;// reload value
   NVIC_ST_CURRENT_R = 0;      // any write to current clears it
   NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
   // enable SysTick with core clock and interrupts
   NVIC_ST_CTRL_R = 0x07;
}

static int count = 0;
int secFlag = 0;
extern int alarm, inAlarm;
const uint16_t sine[32] = {32,38,44,49,54,58,61,62,63,62,61,58,54,49,44,38,32,26,20,15,10,6,3,2,1,2,3,6,10,15,20,26};
uint8_t Index=0;

	
// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
   count++;
   if(count == 100){
      count = 0;
      secFlag = 1;
		  PF2 ^= 0x04; //heartbeat
   }
	 	
	 if(alarm | inAlarm){
			inAlarm = 1;
			GPIO_PORTD_DATA_R = (sine[Index]);    // output one value each interrupt
			Index = (Index+1)% 32;
	 }
}

// Time delay using busy wait.
// The delay parameter is in units of the core clock. (units of 20 nsec for 50 MHz clock)
void SysTick_Wait(uint32_t delay){
  volatile uint32_t elapsedTime;
  uint32_t startTime = NVIC_ST_CURRENT_R;
  do{
    elapsedTime = (startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF;
  }
  while(elapsedTime <= delay);
}

// Time delay using busy wait.
// This assumes 50 MHz system clock.
void SysTick_Wait10ms(uint32_t delay){
  uint32_t i;
  for(i=0; i<delay; i++){
    SysTick_Wait(500000);  // wait 10ms (assumes 50 MHz clock)
  }
}
