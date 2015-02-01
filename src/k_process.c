/**
 * @file:   k_process.c
 * @brief:  process management C file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user processes and NO HARDWARE INTERRUPTS.
 *       The purpose is to show how context switch could be done under stated assumptions.
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations.
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "k_process.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */

/* process initialization table */
PROC_INIT g_proc_table[NUM_TEST_PROCS+1];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];

/**
 * @biref: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 */
void process_init()
{
	int i;
	U32 *sp;

    /* fill out the initialization table */
	set_test_procs();
		printf("gp_pcbs[0] = 0x%x \n", g_test_procs[0].m_priority);
	printf("gp_pcbs[0] = 0x%x \n", g_test_procs[1].m_priority);
	printf("gp_pcbs[0] = 0x%x \n", g_test_procs[2].m_priority);
	printf("gp_pcbs[0] = 0x%x \n", g_test_procs[3].m_priority);
	printf("gp_pcbs[0] = 0x%x \n", g_test_procs[4].m_priority);
	for ( i = 0; i <= NUM_TEST_PROCS; ++i ) {
		g_proc_table[i].m_pid = g_test_procs[i].m_pid;
		g_proc_table[i].m_stack_size = g_test_procs[i].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i].mpf_start_pc;
		
		//printf((char *)g_test_procs[i].m_priority);
		//printf((char *)LOWEST_PRIORITY);
		//printf((char *)HIGHEST_PRIORITY);
		printf("gp_pcbs[0] = 0x%x \n", g_test_procs[i].m_priority);
		if (g_test_procs[i].m_priority > LOWEST_PRIORITY || g_test_procs[i].m_priority < HIGHEST_PRIORITY) {
			g_test_procs[i].m_priority = LOWEST_PRIORITY;
			printf("gp_pcbs[0] = 0x%x \n", g_test_procs[i].m_priority);
		}
		g_proc_table[i].m_priority = g_test_procs[i].m_priority;
	}

	//add null process to proc init table
	g_proc_table[NUM_TEST_PROCS].m_pid = 0;
	g_proc_table[NUM_TEST_PROCS].m_stack_size = USR_SZ_STACK;
	g_proc_table[NUM_TEST_PROCS].mpf_start_pc = &null_proc;
	g_proc_table[NUM_TEST_PROCS].m_priority = LOWEST_PRIORITY + 1;

	/* initilize exception stack frame (i.e. initial context) for each process */
	for ( i = 0; i <= NUM_TEST_PROCS; ++i ) {
		int j;
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->mp_next = NULL;

		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;

		enqueue_priority_queue(ready_queue, gp_pcbs[i], g_proc_table[i].m_priority);
	}
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */

PCB *scheduler(void)
{
	int i;
	PCB *next_proc = NULL;

	//look for next highest process in ready_queue
	for (i = 0; i <= NUM_PRIORITIES; ++i) {
		next_proc = dequeue_priority_queue(ready_queue, i);
		if (next_proc != NULL) {
			return next_proc;
		}
	}

	return next_proc;
}

/*@brief: switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 *@param: p_pcb_old, the old pcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_pcb_old and gp_current_process are pointing to valid PCBs.
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
int process_switch(PCB *p_pcb_old)
{
	PROC_STATE_E state;

	state = gp_current_process->m_state;

	if (state == NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
			p_pcb_old->m_state = RDY;
			p_pcb_old->mp_sp = (U32 *) __get_MSP();
		}
		gp_current_process->m_state = RUN;
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	}

	/* The following will only execute if the if block above is FALSE */

	if (gp_current_process != p_pcb_old) {
		if (state == RDY){
			p_pcb_old->m_state = RDY;
			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			gp_current_process->m_state = RUN;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack
		} else {
			//***************BUG********************
			//queues need some management here
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
		}
	}
	return RTX_OK;
}
/**
 * @brief release_processor().
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
	PCB *p_pcb_old = NULL;

	p_pcb_old = gp_current_process;
	gp_current_process = scheduler();

	if ( gp_current_process == NULL  ) {
		gp_current_process = p_pcb_old; // revert back to the old process
		return RTX_ERR;
	}

	if ( p_pcb_old == NULL ) {
		p_pcb_old = gp_current_process;
	} else {
		enqueue_priority_queue(ready_queue, p_pcb_old, get_process_priority(p_pcb_old->m_pid));
	}

	process_switch(p_pcb_old);
	return RTX_OK;
}

void null_proc(void) {
	while (1){
		k_release_processor();
	}
}
