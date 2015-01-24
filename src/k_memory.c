/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"
#include <stdint.h>

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

#define MEM_BLK_SIZE ((128+32)/4)

typedef struct mem_blk {
	struct mem_blk *next_blk ;
} mem_blk;

/* ----- Global Variables ----- */
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
							/* stack grows down. Fully decremental stack */

/**
 * @brief: Start of the linked list of the memory blocks
 */
mem_blk *start_mem_blk;

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
  
	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += NUM_TEST_PROCS * sizeof(PCB *);
  
	//allocate PCBs
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB); 
	}
#ifdef DEBUG_0  
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
#endif
	
	/* prepare for alloc_stack() to allocate memory for stacks */
	
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}
	
	//allocate memory for ready queue
	ready_queue = (process_queue**)p_end;
	p_end += NUM_PRIORITIES * sizeof(process_queue *);
	
	for ( i = 0; i < NUM_PRIORITIES; i++ ) {
		ready_queue[i] = (process_queue *)p_end;
		p_end += sizeof(process_queue)*NUM_TEST_PROCS;
	}
	initialize_priority_queue(ready_queue);
	
	//allocate memory for blocked queue
	blocked_queue = (process_queue**)p_end;
	p_end += NUM_PRIORITIES * sizeof(process_queue*);
	
	for ( i = 0; i < NUM_PRIORITIES; i++ ) {
		blocked_queue[i] = (process_queue *)p_end;
		p_end += sizeof(process_queue)*NUM_TEST_PROCS;
	}
	initialize_priority_queue(blocked_queue);
	
	/* allocate memory for heap*/
	//initialize memory linked list
	start_mem_blk = (mem_blk*)p_end;
	
	while ( (U32*)(start_mem_blk + MEM_BLK_SIZE) <= gp_stack) {
		start_mem_blk->next_blk = start_mem_blk + MEM_BLK_SIZE;
		start_mem_blk += MEM_BLK_SIZE;
	}
	
	start_mem_blk = (mem_blk*)p_end;  
}

void run_memory_test() 
{
	void *blk; 
	
	//Test one memory block allocation
	mem_blk* start = start_mem_blk;
	blk = k_request_memory_block();
	k_release_memory_block(blk);
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
	return sp;
}

void *k_request_memory_block(void) {
	mem_blk *mem_blk = start_mem_blk;
	
#ifdef DEBUG_0 
	printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */
	//return (void *) NULL;
	
	// atomic ( on ) ;
	
	while ( start_mem_blk == NULL ) {
		//put PCB on blocked_resource_q ;
		//set process state to BLOCKED_ON_RESOURCE ;
		//release_processor ( ) ;
		return NULL;
	}
	
	start_mem_blk = start_mem_blk->next_blk; //update the heap
	mem_blk->next_blk = NULL;
	mem_blk += (uint32_t)1;
	
#ifdef DEBUG_0 
	printf("k_request_memory_block: exiting...\n. mem_blk requested is: 0x%x \nActual mem_blk is: 0x%x \nstart_mem_blk is: 0x%x \n", mem_blk, mem_blk - (uint32_t)1, start_mem_blk);
#endif /* ! DEBUG_0 */
	
	// atomic ( off ) ;
	return mem_blk;
}

int k_release_memory_block(void *p_mem_blk) {
	mem_blk *rel_blk = (mem_blk *)p_mem_blk - (uint32_t)1;
	
#ifdef DEBUG_0 
	printf("k_release_memory_block: releasing block @ 0x%x \n", p_mem_blk);
#endif /* ! DEBUG_0 */
	
	//if ( blocked on resource q not empty ) {
	//	handle_process_ready ( pop ( blocked resource q ) ) ;
	//}
	
	if ( (rel_blk == NULL) || ((U8 *)rel_blk < p_end) || ((U32 *)rel_blk + MEM_BLK_SIZE > gp_stack) || ((int)((U8 *)rel_blk - p_end)%(MEM_BLK_SIZE*4) != 0) ) {
		return RTX_ERR;
	}
	
	rel_blk->next_blk = start_mem_blk;
	start_mem_blk = rel_blk;

#ifdef DEBUG_0 
	printf("\nk_release_memory_block: exiting...\nstart_mem_blk is: 0x%x \n start_mem_blk->next_blk is: 0x%x \n", start_mem_blk, start_mem_blk->next_blk);
#endif /* ! DEBUG_0 */
	
	return RTX_OK;
}

/**
tests:
	-allocate all blocks
	-deallocate all blocks
	-allocate all blocks + 1
	-deallocate null, < p_end, > gp_stack, mid block
	-deallocate block that has not been allocated - only bug (for now...hopefully)
*/
