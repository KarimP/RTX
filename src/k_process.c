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
PCB *uart = NULL; //uart PCB
PCB *timer = NULL;//timer PCB

queue *delayed_queue;

/* process initialization table */
PROC_INIT g_proc_table[NUM_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];

/**
 * @biref: initialize all processes in the system
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
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->mp_next = NULL;
		initialize_queue((gp_pcbs[i])->msg_q );

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

void atomic (int toggle)
{
	if (toggle) {
		__disable_irq();
	}
	else {
		__enable_irq();
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
	PCB *next_proc = gp_current_process;

	//look for next highest process in ready_queue
	for (i = 0; i < NUM_PRIORITIES; ++i) {
		if (ready_queue[i]->first != NULL) {
			next_proc = dequeue_priority_queue(ready_queue, i);
			enqueue_priority_queue(ready_queue, next_proc, i);
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
			priority = k_get_process_priority(gp_current_process->m_pid);

			pop_queue(ready_queue, gp_current_process->m_pid, priority);
			gp_current_process->mp_next = ready_queue[priority]->first;
			ready_queue[priority]->first = gp_current_process;

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

// Message Queue implementation

PCB* get_pcb_from_pid(int process_id)
{
	PCB* pcb;
	int i;
	for (i=0;i < NUM_PROCS; i++){
		if (g_proc_table[i].m_pid == process_id && process_id > 0) {
			pcb = gp_pcbs[i];
		}
	}

	return pcb;
}

void block_process(PCB *proc, int pid, PROC_STATE_E blocked_status)
{
    int proc_priority = k_get_process_priority(pid);
    proc->m_state = blocked_status;
    pop_queue(ready_queue, pid, proc_priority);
    enqueue_priority_queue(blocked_queue, proc, proc_priority);
}

void ready_process(PCB *proc, int pid)
{
    int proc_priority = k_get_process_priority(pid);
    proc->m_state = RDY;
    pop_queue(blocked_queue, pid, proc_priority);
    enqueue_priority_queue(ready_queue, proc, proc_priority);
}

int k_send_message(int receiving_pid, void *message_envelope)
{
	PCB* receiving_proc;
	msg_Node *msg;
	if (message_envelope == NULL)
	{
		return RTX_ERR;
	}

	atomic(ON);

	msg = k_request_memory_block();
	receiving_proc = get_pcb_from_pid(receiving_pid);

	msg->next = NULL;
	msg->d_pid = receiving_pid;
	msg->s_pid = gp_current_process->m_pid;
	msg->msgbuf = message_envelope;

	enqueue(receiving_proc->msg_q, (queue_node*) msg);

	if (receiving_proc->m_state == BLOCKED_ON_RECEIVE) {
		//set state to ready, and move from blocked queue to ready queue
        ready_process(receiving_proc, receiving_pid);

        atomic(OFF);
		k_release_processor();
		atomic(ON);
	}

	atomic(OFF);
	return RTX_OK;
}

int k_send_delayed_message(int process_id, void *message_envelope, int delay)
{
	msg_Node* envelope = (msg_Node*) message_envelope;
	if (message_envelope == NULL)
	{
		return RTX_ERR;
	}
	
	//envelope->expireTime = getCurrentTime() + delay;//is delay in milliseconds? need to compensate if it is
	//envelope->d_pid = process_id;
	//k_send_message(timerPCB->pid ,envelope);//send to the timer process to deal with

	return RTX_OK;
}

void *get_message(int *sender_id, int block)
{
	msg_Node* msg = NULL;
	void *msgbuf = NULL;

	atomic(ON);

	while(isEmpty(gp_current_process->msg_q) && block) {
        block_process(gp_current_process, gp_current_process->m_pid, BLOCKED_ON_RECEIVE);

        atomic(OFF);
		k_release_processor();
		atomic(ON);
	}

	msg = (msg_Node*)dequeue((gp_current_process->msg_q));

	if (msg != NULL) {
		*sender_id = (int)msg->s_pid;
		msgbuf = msg->msgbuf;
		k_release_memory_block((void *) msg);
	}

	atomic(OFF);

	return msgbuf;
}

void *k_receive_message(int *sender_id)
{
	return get_message(sender_id, TRUE);
}

void *k_receive_message_non_block(int *sender_id)
{
	return get_message(sender_id, FALSE);
}

void timer_i_process ( ) {
	/*
	// get pending requests
	while ( pending messages to i-process ) {
		insert envelope into the delayed queue in order of expirery times ;
	}
	while ( first message in queue timeout expired ) {
		msg_t * env = dequeue ( timeout_queue ) ;
		int target_pid = env->destination_pid ;
		// forward msg to destination
		send_message ( target_pid , env ) ;
	}
	*/
}
