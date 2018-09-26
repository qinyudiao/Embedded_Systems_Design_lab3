#include <stdio.h>
#include <stdint.h>
#include "stdlib.h"
#include <math.h>
#include "string.h"
#include "ST7735.h"
#include "tm4c123gh6pm.h"

int32_t min_X, max_X, min_Y, max_Y;

void ST7735_sDecOut2(int32_t n){
    char i_n;
    char f_n;
	if(n > 9999){
	    printf("\n");
		ST7735_OutString(" **.**");
	}
	else if(n < -9999){
		ST7735_OutString("-**.**");
	}
	else{
		  i_n = n / 100;
		  f_n = abs(n) % 100;
		  if( n > -100 && n < 0){
		      printf(" -0.%.2d", f_n);
		  }
		  else{
		      printf("%3d.%.2d", i_n, f_n);
		  }
	}
}


void ST7735_uBinOut6(uint32_t n){
	uint32_t i_n;
    uint32_t f_n;
    uint32_t t_n;
	if(n > 63999){
		ST7735_OutString("***.**");
	}
	else{
	  t_n = n*100/64;
		i_n = t_n / 100;
		if(((n*1000/64) % 10) < 5){
			f_n = t_n % 100;
		}
		else{
			f_n = t_n % 100 + 1;
		}
			printf("%3d.%.2d", i_n, f_n);
	}
}


void ST7735_XYplotInit(char *title, int32_t minX, int32_t maxX, int32_t minY, int32_t maxY){
	ST7735_FillScreen(0);  // set screen to black
  ST7735_SetCursor(0,0);
	ST7735_DrawString(0,0,title, ST7735_WHITE);
	min_X = minX;
	max_X = maxX;
	min_Y = minY;
	max_Y = maxY;
}


void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]){
	for(uint32_t i = 0; i < num; i++){
		if(bufX[i] >= min_X && bufX[i] < max_X && bufY[i] >= min_Y && bufY[i] < max_Y){ //boundary test
			int32_t x = (127*(bufX[i]-min_X)/(max_X-min_X)); // x-coords
			int32_t y = (32 + (127*(max_Y - bufY[i])/(max_Y-min_Y))); // y-coords
			ST7735_DrawPixel(x, y, ST7735_BLUE);
		}
	}
}
