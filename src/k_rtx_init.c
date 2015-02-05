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
        __disable_irq();
        uart0_init();
        memory_init();
        process_init();
				setup_heap();
        __enable_irq();

	/* start the first process */

        k_release_processor();
}
