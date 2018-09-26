// main.h

typedef struct phase_t {
	char *options[5];
	int time[3];
	int8_t  highlight;    // index (out of total)l; -1 for none
	int color[6];     
	uint8_t selected;      // 0 for not selected. 1 for selected
} phase;
