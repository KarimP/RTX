/**
 * @file:   k_rtx_init.c
 * @brief:  Kernel initialization C file
 * @auther: SE350 G017
 * @date:   2015/02/03
 */

#include "k_rtx_init.h"
#include "uart_polling.h"
#include "k_memory.h"
#include "k_process.h"
#include "timer.h"

void k_rtx_init(void)
{
    atomic(ON);

    uart0_irq_init(); //irq, for console input
    uart0_init(); //polling, for DEBUGGING

    timer_init(0);
    memory_init();
    process_init();
    setup_heap();
    atomic(OFF);

		/* start the first process */
    k_release_processor();
}
