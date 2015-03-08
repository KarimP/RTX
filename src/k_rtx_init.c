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

void k_rtx_init(void)
{
    atomic(ON);
	//timer_init(0);

    uart1_irq_init(); // interrupt driven, for RTX console
    uart0_init(); //polling, for DEBUGGING

    memory_init();
    process_init();
    setup_heap();
    atomic(OFF);

	/* start the first process */
    k_release_processor();
}
