/**
 * @brief timer.h - Timer header file
 * @author Y. Huang
 * @date 2013/02/12
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#define DELAY_MS 1
#define CLOCK_SIZE 11

//states of g_timer counter used by timer 1 for timing analysis
typedef enum {
    STOPPED = 0, RUNNING, PAUSED
} TIMER_STATE;

extern uint32_t timer_init(uint8_t n_timer);  /* initialize timer n_timer */

int start_timer(void);
int pause_timer(void);
void stop_timer(void);
void continue_timer(void);

#endif /* ! _TIMER_H_ */
