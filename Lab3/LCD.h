// LCD.h
#define HOURLENGTH 15
#define MINUTELENGTH 20
#define CENTERX 61
#define CENTERY 100
#define PI 3.14159

//************* ST7735_Line********************************************
//  Draws one line on the ST7735 color LCD
//  Inputs: (x1,y1) is the start point
//          (x2,y2) is the end point
// the relative positions of (x1,y1) and (x2,y2) need not to be concerned
// x1,x2 are horizontal positions, columns from the left edge
//               must be less than 128
//               0 is on the left, 127 is near the right
// y1,y2 are vertical positions, rows from the top edge
//               must be less than 160
//               159 is near the wires, 0 is the side opposite the wires
//        color 16-bit color, which can be produced by ST7735_Color565() 
// Output: none

void ST7735_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/* draws the 4 surrounding pixels of a given point, making the line thicker */
void ST7735_4Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void drawFace(void);

void eraseHands(int time);

void drawHands(int time);
