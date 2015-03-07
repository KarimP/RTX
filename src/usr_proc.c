/**
 * @file:   usr_proc.c
 * @brief:  Two user processes: proc1 and proc2
 * @author: SE 350 G017
 * @date:   2015/02/03
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

#define TRUE 1
#define FALSE 0

#define PROC1_PID 1
#define PROC2_PID 2
#define PROC_MEM_PID 3
#define PROC_PRIORITY_PID 4
#define PROC_PREEMPTION_PID 5
#define NUM_TESTS 4

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];
int testCounter = 1;
int passedtest = 0;

int process4_preempted = FALSE;
int testing_priority = FALSE;

void set_up_testing_statements() {

	#ifdef DEBUG_0
	printf("G017_test: START\n\r");
	printf("G017_test: total %x tests\n\r", + NUM_TESTS);
	#endif /* DEBUG_0 */
}

void printTestResults(int outcome) {
	if(testCounter > NUM_TESTS) {

		#ifdef DEBUG_1
		printf("Test: %x\n\r", testCounter);
		#endif /* DEBUG_1 */

		testCounter++;
		return;
	}

	if(outcome == 1) {
		passedtest++;
	}

	#ifdef DEBUG_0
	printf("G017_test: test %x %s\n\r", testCounter, outcome == 1? "OK" : "FAIL");
	#endif /* DEBUG_0 */

	testCounter++;
	if (testCounter > NUM_TESTS) {

		#ifdef DEBUG_0
		printf("G017_test: %x/%x tests OK\n\r", passedtest, NUM_TESTS);
		#endif /* DEBUG_0 */

		if (passedtest!= NUM_TESTS) {

			#ifdef DEBUG_0
			printf("G017_test: %x/%x tests FAIL\n\r", (NUM_TESTS - passedtest),NUM_TESTS);
			#endif /* DEBUG_0 */

		}

		#ifdef DEBUG_0
		printf("G017_test: END\n\r");
		#endif /* DEBUG_0 */
	}
}

void set_test_procs() {
	int i;
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_priority=MEDIUM;
		g_test_procs[i].m_stack_size=0x100;
	}

	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[2].mpf_start_pc = &memory_management_test;
	g_test_procs[3].mpf_start_pc = &priority_test;
	g_test_procs[4].mpf_start_pc = &preemption_check;
	g_test_procs[5].mpf_start_pc = &proc6;

	set_up_testing_statements();
}

/**
 * @brief: Process 1: a process that prints five uppercase letters
 *         and then yields the cpu.
 */
void proc1(void)
{
	int i = 0;
    int sender_id = -1;
	MSG_BUF *msg = NULL;
	while (TRUE) {
        msg = (MSG_BUF *)receive_message(&sender_id);

		printf("message received from sender: %d Message: ", sender_id);
		for (i = 0; i < 4; ++i) {
			printf("%c", msg->mtext[i]);
		}
        uart0_put_string("\n\r");
        release_memory_block(msg);

        printTestResults(TRUE);
        release_processor();
	}
}

/**
 * @brief: Process 2: a process that prints five numbers
 *         and then yields the cpu.
 */
void proc2(void)
{
	MSG_BUF *msg;
	while (TRUE) {

		void *blk = request_memory_block();

        msg = (MSG_BUF *)request_memory_block();
		msg->mtype = DEFAULT;
		msg->mtext[0] = 'h';
		msg->mtext[1] = 'e';
		msg->mtext[2] = 'l';
		msg->mtext[3] = 'l';

		send_message(PROC1_PID, msg);

        printTestResults(TRUE);
        release_processor();
	}
}

/**
 * @brief: Process 3: check if memory management works
 */
void memory_management_test(void)
{
	while (TRUE) {

		//	allocate some blocks of memory
	 	void *mem_blks[3];
	 	int num_blks = 3;
	 	int passed = TRUE;
		int i, j;

		for (i = 0; i < num_blks; ++i) {
			mem_blks[i] = request_memory_block();
		}

		//ensure that memory blocks are unique
		for (i = 0; i < num_blks; ++i) {
			for (j = 0; j < num_blks; ++j) {
				if ((j != i) && (mem_blks[i] == mem_blks[j])) {
					passed = FALSE;
					break;
				}
			}

			if (!passed) {
				break;
			}
		}

	 	//deallocate all blocks but 1
		for (i = 0; i < num_blks-1; ++i) {
			if (release_memory_block(mem_blks[i]) == RTX_ERR) {
	 			passed = FALSE;
	 		}
		}

	 	//deallocate block that has not been allocated
	 	if (release_memory_block(mem_blks[1]) != RTX_ERR) {
	 		passed = FALSE;
	 	}

	 	//deallocate null block
	 	if (release_memory_block(NULL) != RTX_ERR) {
	 		passed = FALSE;
	 	}

	 	//deallocate address greater than the heap
	 	if (release_memory_block((void *)0x10008000) != RTX_ERR) {
	 		passed = FALSE;
	 	}

	 	//deallocate address less than the heap
	 	if (release_memory_block((void *)0x10000000) != RTX_ERR) {
	 		passed = FALSE;
	 	}

	 	//deallocate address that is the end of a memory block
	 	if (release_memory_block((int *)mem_blks[num_blks - 1] - (uint32_t)1) != RTX_ERR) {
	 		passed = FALSE;
	 	}

		//deallocate address that is the middle of a memory block
	 	if (release_memory_block((int *)mem_blks[num_blks - 1] - (uint32_t)3) != RTX_ERR) {
	 		passed = FALSE;
	 	}

	 	//deallocate the last held memory block
	 	if (release_memory_block(mem_blks[num_blks - 1]) == RTX_ERR) {
	 		passed = FALSE;
	 	}

	 	#ifdef DEBUG_1
	 	printf("testing memory blocks...%s \n\r", passed ? "passed" : "failed");
	 	#endif /* DEBUG_1 */
		printTestResults(passed);

	 	release_processor();
	 }
}

/**
 * @brief: Process 4: check if handling of processes works (get and set priority queues)
 */
void priority_test(void)
{
	while (TRUE) {

		void *blk = request_memory_block();
		int passed = TRUE;
		testing_priority = TRUE;

		//check to see if we can change null process priority
		if (set_process_priority(0, HIGH) == RTX_OK) {
			passed = FALSE;
		}

		//make sure it actually didn't end up changed the null proc priority
		if (get_process_priority(0) != LOWEST + 1) {
			passed = FALSE;
		}

		//set this processes' priority to the same one
		if (set_process_priority(PROC_PRIORITY_PID, MEDIUM) == RTX_ERR) {
			passed = FALSE;
		}

		//set this processes' priority to a higher one
		if (set_process_priority(PROC_PRIORITY_PID, HIGH) == RTX_ERR) {
			passed = FALSE;
		}

		//go back to medium
		if (set_process_priority(PROC_PRIORITY_PID, MEDIUM) == RTX_ERR) {
			passed = FALSE;
		}

		//set non-current process priority lower than current
		if (set_process_priority(PROC_PREEMPTION_PID, LOW) == RTX_ERR) {
			passed = FALSE;
		}

		if (!passed) {
			// printTestResults(passed);
			testing_priority = FALSE;
			release_processor();
		}

		//set another processes priority to higher than this one
		process4_preempted = TRUE;
		if (set_process_priority(PROC_PREEMPTION_PID, HIGH) == RTX_ERR) {
			testing_priority = FALSE;
			release_processor();
		}
	}
}

/**
 * @brief: Process 5: check if pre-emption occured with pid 4
 */
void preemption_check(void)
{
	while (TRUE) {

		void *blk = request_memory_block();
		int passed = TRUE;

		if (testing_priority) {
			testing_priority = FALSE;

			if (process4_preempted) {
				process4_preempted = FALSE;
			} else {
				passed = FALSE; //we pre-empted when we weren't supposed to
			}

			if (set_process_priority(PROC_PREEMPTION_PID, MEDIUM) == RTX_ERR) {
				passed = FALSE;
			}

		} else {
			passed = FALSE;
		}

		printTestResults(passed);
		release_processor();
	}
}

/**
 * @brief: Process 6: Allocates all blocks of memory
*/
void proc6(void)
{

	while (TRUE) {
		request_memory_block();
	}
}
