/**
 * @file:   k_process.c
 * @brief:  process management C file
 * @author: SE350 G017
 * @author: Thomas Reidemeister
 * @date:   2015/02/03
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

/* process priority queues */
queue** ready_queue;
queue** blocked_queue;

/* process initialization table */
PROC_INIT g_proc_table[NUM_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];

/**
 * @brief: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 */
void process_init()
{
	int i;
	U32 *sp;
	
	//get kernel procs
	k_set_procs();

    /* fill out the initialization table */
	set_test_procs();

	for ( i = NUM_K_PROCS; i < NUM_PROCS; i++ ) {
		g_proc_table[i].m_pid = g_test_procs[i-1].m_pid;
		g_proc_table[i].m_stack_size = g_test_procs[i-1].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i-1].mpf_start_pc;

		//change priority to lowest if its out of bounds so that the user process runs
		if (g_test_procs[i-1].m_priority > LOWEST_PRIORITY || g_test_procs[i-1].m_priority < HIGHEST_PRIORITY) {
			g_test_procs[i-1].m_priority = LOWEST_PRIORITY;
		}
		g_proc_table[i].m_priority = g_test_procs[i-1].m_priority;
	}

	/* initilize exception stack frame (i.e. initial context) for each process */
	for ( i = 0; i < NUM_PROCS; i++ ) {
		int j;
		int priority = g_proc_table[i].m_priority;
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->next = NULL;

		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;

		enqueue(ready_queue[priority], (queue_node *)gp_pcbs[i]);
	}
}

int process_pid_compar(const queue_node *node, const void *property) {
	return ( ((PCB*)node)->m_pid == *((int *)property) );
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
	PCB *next_proc = gp_current_process;

	//look for next highest process in ready_queue
	for (i = 0; i < NUM_PRIORITIES; ++i) {
		if (ready_queue[i]->first != NULL) {
			next_proc = (PCB *)ready_queue[i]->first;
			rotate_queue(ready_queue[i]);
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
	int priority;

	state = gp_current_process->m_state;

	if (state == NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
			if (p_pcb_old->m_state == RUN) {
				p_pcb_old->m_state = RDY;
			}

			p_pcb_old->mp_sp = (U32 *) __get_MSP();
		}
		gp_current_process->m_state = RUN;
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	}

	/* The following will only execute if the if block above is FALSE */
	if (gp_current_process != p_pcb_old) {
		if (state == RDY){
			if (p_pcb_old->m_state == RUN) {
				p_pcb_old->m_state = RDY;
			}

			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			gp_current_process->m_state = RUN;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack
		} else {
			//move current process back to front of its queue so it'll be chosen next
			priority = k_get_process_priority(gp_current_process->m_pid);

			remove_node(ready_queue[priority], &process_pid_compar, (void *)&gp_current_process->m_pid);
			gp_current_process->next = (PCB *)ready_queue[priority]->first;
			ready_queue[priority]->first = (queue_node *)gp_current_process;
			
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
	}

	process_switch(p_pcb_old);
	return RTX_OK;
}

int k_set_process_priority(int process_id, int priority)
{
	int i = 0;
	int current_process_priority = 0;
	int prev_priority = -1;

	queue_node *proc = NULL;
	queue** q = ready_queue;

	if (process_id == 0 || priority > LOWEST_PRIORITY || priority < HIGHEST_PRIORITY) {
		return RTX_ERR;
	}

	//get current process and given process priority
	for (i = 0; i < NUM_PROCS; ++i) {
		if (g_proc_table[i].m_pid == process_id) {
			prev_priority = g_proc_table[i].m_priority;
			g_proc_table[i].m_priority = priority;
		}

		if (g_proc_table[i].m_pid == gp_current_process->m_pid) {
			current_process_priority = g_proc_table[i].m_priority;
		}
	}

	//didn't find process in process table
	if (prev_priority == -1) {
		return RTX_ERR;

	//process has same priority, do nothing
	} else if (prev_priority == priority) {
		return RTX_OK;
	}

	proc = remove_node(q[prev_priority], &process_pid_compar, (void *)&process_id);
	if (proc == NULL) {
		q = blocked_queue;
		proc = remove_node(q[prev_priority], &process_pid_compar, (void *)&process_id);
	}

	enqueue(q[priority], proc);

	if (current_process_priority > highest_filled_priority(ready_queue)) {
		k_release_processor();
	}

	return RTX_OK;
}


int k_get_process_priority(int process_id)
{
	int i;
	for (i = 0; i < NUM_PROCS; ++i) {
		if (g_proc_table[i].m_pid == process_id) {
			return g_proc_table[i].m_priority;
		}
	}

	return RTX_ERR;
}
