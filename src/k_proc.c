/**
 * @file:   usr_proc.c
 * @brief:  Two user processes: proc1 and proc2
 * @author: SE 350 G017
 * @date:   2015/02/03
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

#include "k_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

void k_set_procs(void) {
	int i = 0;

	for (i = 0; i < NUM_K_PROCS; ++i) {
		g_proc_table[i].m_pid=(U32)(i);
		g_proc_table[i].m_priority=1;
		g_proc_table[i].m_stack_size=USR_SZ_STACK;
	}

	//set null process
	g_proc_table[0].m_priority=4;
	g_proc_table[0].mpf_start_pc=&null_proc;

	//set other kernel processes
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
