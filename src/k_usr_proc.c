/**
 * @file:   usr_proc.c
 * @brief:  Two user processes: proc1 and proc2
 * @author: SE 350 G017
 * @date:   2015/02/03
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

#include "k_usr_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* initialization table item */
extern PROC_INIT g_proc_table[NUM_PROCS];

void k_set_procs(void) {
	//set null process
	g_proc_table[0].m_pid=(U32)(0);
	g_proc_table[0].m_priority=4;
	g_proc_table[0].m_stack_size=0x100;
	g_proc_table[0].mpf_start_pc=&null_proc;
}

/**
 * @brief: null process
 */
void null_proc(void)
{
	while (1) {

		#ifdef DEBUG_1
		printf("null process running\n\r");
		#endif /* DEBUG_1 */

		k_release_processor();
	}
}
