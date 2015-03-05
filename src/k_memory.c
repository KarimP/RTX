/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: SE350 G017
 * @date:   2015/02/03
 */

#include "k_memory.h"
#include <stdint.h>

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

#define MEM_BLK_SIZE 128
#define NUM_BLOCKS 20

/* ----- Global Variables ----- */

/* @brief The last allocated stack low address. 8 bytes aligned.
 * 		  The first stack starts at the RAM high address and
 *    	  stack grows down. Fully decremental stack.
 */
 U32 *gp_stack;

/**
 * @brief: Start of the linked list of the memory blocks
 */
queue *mem_q;

/**
 * @brief: Start of the heap allocated for memory blocks
 */
U8 *p_end;

/**
 * @brief: Initialize RAM as follows:
0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|<--- p_end
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address
*/

void memory_init(void)
{
	int i;
	p_end = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;

	#ifdef DEBUG_1
	printf("p_end = 0x%x \n\r", p_end);
	#endif /* ! DEBUG_1 */

	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers */
	gp_pcbs = (PCB **)p_end;
	p_end += NUM_PROCS * sizeof(PCB *);

	//allocate for PCBs
	for ( i = 0; i < NUM_PROCS; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB);
	}

	#ifdef DEBUG_1
	printf("gp_pcbs[0] = 0x%x \n\r", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n\r", gp_pcbs[1]);
	printf("gp_pcbs[2] = 0x%x \n\r", gp_pcbs[2]);
	printf("gp_pcbs[3] = 0x%x \n\r", gp_pcbs[3]);
	printf("gp_pcbs[4] = 0x%x \n\r", gp_pcbs[4]);
	printf("gp_pcbs[5] = 0x%x \n\r", gp_pcbs[5]);
	printf("gp_pcbs[6] = 0x%x \n\r", gp_pcbs[6]);
	#endif /* ! DEBUG_1 */

	/* prepare for alloc_stack() to allocate memory for stacks */
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack;
	}

	//allocate memory for ready queue
	ready_queue = (queue**)p_end;

	#ifdef DEBUG_1
	printf("Start of ready_queue array: 0x%x \n\r", ready_queue);
	#endif /* ! DEBUG_1 */

	p_end += (NUM_PRIORITIES) * sizeof(queue*);

	for ( i = 0; i < NUM_PRIORITIES; i++ ) {
		ready_queue[i] = (queue*)p_end;

		#ifdef DEBUG_1
		printf("ready_queue[%d]: 0x%x \n\r", i, ready_queue[i]);
		#endif /* ! DEBUG_1 */

		p_end += sizeof(queue);
	}
	init_priority_queue(ready_queue);

	//allocate memory for blocked queue
	blocked_queue = (queue**)p_end;

	#ifdef DEBUG_1
	printf("Start of blocked_queue array: 0x%x \n\r", blocked_queue);
	#endif /* ! DEBUG_1 */

	p_end += NUM_PRIORITIES * sizeof(queue*);

	for ( i = 0; i < NUM_PRIORITIES; i++ ) {
		blocked_queue[i] = (queue *)p_end;

		#ifdef DEBUG_1
		printf("blocked_queue[%d]: 0x%x \n\r", i, blocked_queue[i]);
		#endif /* ! DEBUG_1 */

		p_end += sizeof(queue);
	}
	init_priority_queue(blocked_queue);
}

/**
 * @brief allocates all of the remaining heap memory and sets up the memory queue
 * @return RTX_ERR on error and RTX_OK on success
 */
int setup_heap(void)
{
	mem_blk blk;
	int counter = 0;
	int i;

  	// allocate memory for heap memory queue
	mem_q = (queue*)p_end;
	p_end += sizeof(queue);

	//initialize memory linked list
	init_queue(mem_q);
	blk = (mem_blk)p_end;

	for (i = 0; i < NUM_BLOCKS; i++){
		enqueue(mem_q, blk);
		blk = (mem_blk)((char *)blk + MEM_BLK_SIZE + sizeof(queue_node));
		counter++;
	}

	#ifdef DEBUG_1
	printf("We have %d memory blocks\n\r", counter);
	printf("mem_q->first = 0x%x \n\r", mem_q->first);
	printf("mem_q->last = 0x%x \n\r", mem_q->last);
	printf("gpstack b= 0x%x \n\r", gp_stack);
	#endif /* ! DEBUG_1 */

	if (mem_q->first == (mem_blk)p_end){
		return RTX_OK;
	} else {
		return RTX_ERR;
	}
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

U32 *alloc_stack(U32 size_b)
{
	U32 *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */

	/* update gp_stack */
	gp_stack = (U32 *)((U8 *)sp - size_b);

	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack;
	}

	#ifdef DEBUG_1
	printf("gpstack = 0x%x \n\r", gp_stack);
	#endif /* ! DEBUG_1 */

	return sp;
}

/**
 * @brief attempts to allocate a memory block for the process and blocks the process if that is not possible
 * @return returns a memory block when it is possible to do so
 */
void *k_request_memory_block(void)
{
	int process_priority = 0;
	mem_blk blk = dequeue(mem_q);

	#ifdef DEBUG_1
	printf("k_request_memory_block: entering...\n\r");
	#endif /* ! DEBUG_1 */

	while (blk == NULL) {

		process_priority = k_get_process_priority(gp_current_process->m_pid);
		remove_node(ready_queue[process_priority], &process_pid_compar, (void *) &gp_current_process->m_pid);
		enqueue(blocked_queue[process_priority], (queue_node *)gp_current_process);

		//set process state to BLOCKED_ON_RESOURCE ;
		gp_current_process->m_state = BLOCKED_ON_RESOURCE;

		k_release_processor();
	}

	#ifdef DEBUG_1
	printf("k_request_memory_block: exiting...\n\rblk requested is: 0x%x \nReturned blk is: 0x%x \nmem_q->first is: 0x%x \n\r\n\r", blk, blk + 1, mem_q->first);
	#endif /* ! DEBUG_1 */

	return blk + 1;
}

/**
 * @brief deallocates the passed in memory block if it is possible to do so
 * @return RTX_ERR on error and RTX_OK on success
 */
int k_release_memory_block(void *p_mem_blk)
{
	mem_blk blk = (mem_blk)p_mem_blk - 1;
	int i = 0;
	PCB *pcb = NULL;

	#ifdef DEBUG_1
	printf("k_release_memory_block: releasing block @ 0x%x \n\r", p_mem_blk);
	#endif /* ! DEBUG_1 */

	int a = (blk == NULL);
	
	//make sure block is not below heap
	int b = ((U8 *)blk < p_end);
	
	//make sure block doesn't touch stack
	int c = ((U8 *)blk + (MEM_BLK_SIZE + sizeof(queue_node)) > (U8*)gp_stack); 
	
	//make sure block address matches an address that would be at the beginning of the block
	int d = ((int)((U8 *)blk - p_end)%(MEM_BLK_SIZE + sizeof(queue_node)) != 0); 
	
	if ( a || b || c || d ) {
		return RTX_ERR;
	}

	if (enqueue(mem_q, blk) != RTX_OK) {
		return RTX_ERR;
	}

	// remove highest priority blocked process that requested memory previously
	for (i = 0; i < NUM_PRIORITIES; ++i) {
		pcb = (PCB *)dequeue(blocked_queue[i]);
		if (pcb != NULL) {
			pcb->m_state = RDY;
			enqueue(ready_queue[i], (queue_node *)pcb);

			if (i > k_get_process_priority(gp_current_process->m_pid)) {
				k_release_processor();
			}
			break;
		}
	}

	#ifdef DEBUG_1
	printf("k_release_memory_block: exiting...\n\rmem_q->first is: 0x%x \nmem_q->first->next is: 0x%x \n\r\n\r", mem_q->first, mem_q->first->next);
	#endif /* ! DEBUG_1 */

	return RTX_OK;
}
