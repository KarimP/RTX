#ifndef WALLCLOCK_H_
#define WALLCLOCK_H_

#include "rtx.h"
#include <LPC17xx.h>

#define SECOND 10

void wall_clock_proc(void);
int to_int (uint8_t);
void display_time(int clock_count);

#endif
