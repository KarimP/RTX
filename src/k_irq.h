#ifndef K_IRQ_H_
#define K_IRQ_H_

typedef enum { UART = 0, TIMER } irq_type;

void k_irq_handler(irq_type);

#endif /* ! K_IRQ_H_ */
