/**
 * @brief timer.h - Timer header file
 * @author Y. Huang
 * @date 2013/02/12
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#define DELAY_MS 1
#define CLOCK_SIZE 11

extern uint32_t timer_init ( uint8_t n_timer );  /* initialize timer n_timer */
int get_current_time (void);
int get_wall_time (void);
void reset_timer(void);
void set_timer (int, int, int);
void format_time(char*);
extern int error;

#endif /* ! _TIMER_H_ */
