#ifndef K_IRQ_H_
#define K_IRQ_H_

#include <LPC17xx.h>
typedef enum { UART = 0, TIMER } irq_type;

void init_irq_procs(void);

void k_irq_handler(irq_type);

void timer_irq_proc(void);
void uart_irq_proc(char);

#endif /* ! K_IRQ_H_ */
