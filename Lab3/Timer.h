// Timer.h

int updateTime(int flag, int time);

void outputTime(int time, uint8_t cursor_num);		//output current time
void outputTimer(int time, uint8_t cursor_num);		//output stop watch
void getSeconds(int time, char secStr[2]);
void getMinutes(int time, char minStr[2]);
void getHours(int time, char hourStr[2]);
