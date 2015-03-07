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
    uart0_init();
    memory_init();
    process_init(); // need to init the two iprocesses as well
	setup_heap();
    atomic(OFF);

	/* start the first process */
    k_release_processor();
}
