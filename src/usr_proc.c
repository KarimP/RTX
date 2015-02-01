/**
 * @file:   usr_proc.c
 * @brief:  Two user processes: proc1 and proc2
 * @author: Yiqing Huang
 * @date:   2014/01/17
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */


/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];

void set_test_procs() {
	int i;
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_priority=LOW;
		g_test_procs[i].m_stack_size=0x100;
	}

	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[3].mpf_start_pc = &proc3;
	g_test_procs[2].mpf_start_pc = &proc4;
}

/**
 * @brief: a process that prints five uppercase letters
 *         and then yields the cpu.
 */
void proc1(void)
{
	int i = 0;
	int ret_val = 10;
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart0_put_string("\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc1: ret_val=%d\n", ret_val);
#endif /* DEBUG_0 */
		}
		uart0_put_char('A' + i%26);
		i++;
	}
}

/**
 * @brief: a process that prints five numbers
 *         and then yields the cpu.
 */
void proc2(void)
{
	int i = 0;
	int ret_val = 20;
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart0_put_string("\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc2: ret_val=%d\n", ret_val);
#endif /* DEBUG_0 */
		}
		uart0_put_char('0' + i%10);
		i++;
	}
}

/**
 * @brief: check if handling of processes works (get and set priority queues)
 */
void proc4(void)
{
	while (1) {
		release_processor();
	}
}

/**
 * @brief: check if memory management works
 */
void proc3(void)
{
	int i = 0;
	int ret_val = 20;
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart0_put_string("\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc2: ret_val=%d\n", ret_val);
#endif /* DEBUG_0 */
		}
		uart0_put_char('0' + i%10);
		i++;
	}
// 	//allocate some blocks of memory
// 	void *mem_blks[10];
// 	int num_blks = 10;
// 	int passed = 1;

// 	while (num_blks > 0) {
// 		num_blks--;
// 		mem_blks[num_blks] = request_memory_block();
// 	}

// 	//deallocate all blocks but 1
// 	num_blks = 9;
// 	while (num_blks > 0) {
// 		num_blks--;
// 		if (release_memory_block(mem_blks[num_blks]) == RTX_ERR) {
// 			passed = 0;
// 		}
// 	}

// 	//deallocate block that has not been allocated
// 	if (release_memory_block(mem_blks[1]) != RTX_ERR) {
// 		passed = 0;
// 	}

// 	//deallocate null block
// 	if (release_memory_block(NULL) != RTX_ERR) {
// 		passed = 0;
// 	}

// 	//deallocate address greater than the heap
// 	if (release_memory_block((void *)0x10008000) != RTX_ERR) {
// 		passed = 0;
// 	}

// 	//deallocate address less than the heap
// 	if (release_memory_block((void *)0x10000000) != RTX_ERR) {
// 		passed = 0;
// 	}

// 	//deallocate address that is the end of a memory block
// 	if (release_memory_block((int *)mem_blks[0] - (uint32_t)1) != RTX_ERR) {
// 		passed = 0;
// 	}

// 	//deallocate address that is the middle of a memory block
// 	if (release_memory_block((int *)mem_blks[0] - (uint32_t)3) != RTX_ERR) {
// 		passed = 0;
// 	}

// 	//deallocate the last held memory block
// 	if (release_memory_block(mem_blks[0]) == RTX_ERR) {
// 		passed = 0;
// 	}

// #ifdef DEBUG_0
// 	printf("testing memory blocks...%s \n", passed ? "passed" : "failed");
// #endif /* DEBUG_0 */

// 	release_processor();
}

/**
tests:
	-allocate all blocks
	-deallocate all blocks
	-allocate all blocks + 1
	-deallocate null, < p_end, > gp_stack, mid block
	-deallocate block that has not been allocated
*/
