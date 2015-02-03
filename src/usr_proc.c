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
int testCounter = 1;
int passedtest = 0;


void set_up_testing_statements() {
	printf("G017_test: START\n");
	printf("G017_test: total %x tests\n", + NUM_TEST_PROCS);
}

void printTestResults(int outcome) {
	if(testCounter > NUM_TEST_PROCS) {
		printf("Test: %x\n", testCounter);
		testCounter++;
		return;
	}

	if(outcome == 1) {
		passedtest++;
	}
	printf("G017_test: test %x %s\n", testCounter, outcome == 1? "OK" : "FAIL");
	testCounter++;
	if(testCounter > NUM_TEST_PROCS) {
		printf("G017_test: %x/%x tests OK\n", passedtest, NUM_TEST_PROCS);
		if(passedtest!= NUM_TEST_PROCS) {
			printf("G017_test: %x/%x tests FAIL\n", (NUM_TEST_PROCS - passedtest),NUM_TEST_PROCS);
		}
		printf("G017_test: END\n");
	}
}

void set_test_procs() {
	int i;
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_priority=LOW;
		g_test_procs[i].m_stack_size=0x100;
	}

	//g_test_procs[3].m_priority = LOW;
	//g_test_procs[4].m_priority = LOWEST;

	// printf("g_test_procs[0] = 0x%x \n", g_test_procs[0].m_priority);
	// printf("g_test_procs[1] = 0x%x \n", g_test_procs[1].m_priority);
	// printf("g_test_procs[2] = 0x%x \n", g_test_procs[2].m_priority);
	// printf("g_test_procs[3] = 0x%x \n", g_test_procs[3].m_priority);
	// printf("---------------------- \n");

	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[4].mpf_start_pc = &proc5;

	set_up_testing_statements();
	// printf("g_test_procs[0] = 0x%x \n", g_test_procs[0].m_priority);
	// printf("g_test_procs[1] = 0x%x \n", g_test_procs[1].m_priority);
	// printf("g_test_procs[2] = 0x%x \n", g_test_procs[2].m_priority);
	// printf("g_test_procs[3] = 0x%x \n", g_test_procs[3].m_priority);
	// printf("---------------------- \n");
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
			printTestResults(1);
			ret_val = release_processor();
// #ifdef DEBUG_0
// 			printf("proc1: ret_val=%d\n", ret_val);
// #endif /* DEBUG_0 */
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
			printTestResults(0);
			ret_val = release_processor();
// #ifdef DEBUG_0
// 			printf("proc2: ret_val=%d\n", ret_val);
// #endif /* DEBUG_0 */
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
		printf("proc4: ret \n");
		printTestResults(1);
		release_processor();
	}
}

/**
 * @brief: check if handling of processes works (get and set priority queues)
 */
void proc5(void)
{
	while (1) {
		printf("proc5: ret \n");
		printTestResults(1);
		release_processor();
	}
}

void proc6(void)
{
	while (1) {
		printf("proc6: ret \n");
		printTestResults(1);
		release_processor();
	}
}

/**
 * @brief: check if memory management works
 */
void proc3(void)
{
	while (1) {
	//	allocate some blocks of memory
	 	void *mem_blks[8];
	 	int num_blks = 8;
	 	int passed = 1;
		int i;

		for (i = 0; i < num_blks; ++i) {
			mem_blks[i] = request_memory_block();
		}

	 	//deallocate all blocks but 1
		for (i = 0; i < num_blks-1; ++i) {
			if (release_memory_block(mem_blks[i]) == RTX_ERR) {
	 			passed = 0;
	 		}
		}

	 	//deallocate block that has not been allocated
	 	if (release_memory_block(mem_blks[1]) != RTX_ERR) {
	 		passed = 0;
	 	}

	 	//deallocate null block
	 	if (release_memory_block(NULL) != RTX_ERR) {
	 		passed = 0;
	 	}

	 	//deallocate address greater than the heap
	 	if (release_memory_block((void *)0x10008000) != RTX_ERR) {
	 		passed = 0;
	 	}

	 	//deallocate address less than the heap
	 	if (release_memory_block((void *)0x10000000) != RTX_ERR) {
	 		passed = 0;
	 	}

	 	//deallocate address that is the end of a memory block
	 	if (release_memory_block((int *)mem_blks[num_blks - 1] - (uint32_t)1) != RTX_ERR) {
	 		passed = 0;
	 	}

		//deallocate address that is the middle of a memory block
	 	if (release_memory_block((int *)mem_blks[num_blks - 1] - (uint32_t)3) != RTX_ERR) {
	 		passed = 0;
	 	}

	 	//deallocate the last held memory block
	 	if (release_memory_block(mem_blks[num_blks - 1]) == RTX_ERR) {
	 		passed = 0;
	 	}

	 #ifdef DEBUG_0
	 	printf("testing memory blocks...%s \n", passed ? "passed" : "failed");
	 #endif /* DEBUG_0 */
		printTestResults(passed);

	 	release_processor();
	 }
}

/**
tests:
	MEMORY
	-allocate all blocks
	-deallocate all blocks
	-allocate all blocks + 1
		-ensure process gets blocked
		-last allocated kept in a global variable, another process releases that block
	-deallocate null, < p_end, > gp_stack, mid block - DONE
	-deallocate block that has not been allocated - DONE

	SCHEDULAR
	-set current process priority to higher when highest
	-set current process priority to lower but still highest
	-set current process priority to lower than next-highest
	-set non-current process priority to higher than current
	-set non-current process priority higher but lower than current
	-set non-current process priority lower than current
	-block all processes to ensure null is running
	-block all user processes but one <- unblocked one will get called multiple times back to back

	ideas for some proc tests:
	one test proc that continuously requests memory till all is gone
	one that verifies unique memory blocks are returns upon request
	one test proc which requests, releases, and requests again gets the same memory block
	one test proc which checks that null proc priority cannot be changed
	one test proc that only requests one memory block and then releases processor
	one test proc to use set_proc_priority to manipulate flow of procs
	**investigate SVC interrupts
*/
