/**
 * @file:   usr_proc.c
 * @brief:  Two user processes: proc1 and proc2
 * @author: SE 350 G017
 * @date:   2015/02/03
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */
#include "usr_proc.h"


/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];
int testCounter = 1;
int passedtest = 0;

int process4_preempted = FALSE;
int testing_priority = FALSE;
int process_6_blocked = FALSE;
void *mem_blks[NUM_BLOCKS];

void set_up_testing_statements() {

	#ifdef DEBUG_0
	printf("G017_test: START\n\r");
	printf("G017_test: total %x tests\n\r", + NUM_TESTS);
	#endif /* DEBUG_0 */
}

void printTestResults(int outcome) {
	if (testCounter > NUM_TESTS) {
		#ifdef DEBUG_1
		printf("Test: %x\n\r", testCounter++);
		#endif /* DEBUG_1 */

		return;
	}

	if (outcome == 1) {
		passedtest++;
	}

	#ifdef DEBUG_0
	printf("G017_test: test %x %s\n\r", testCounter++, outcome == 1? "OK" : "FAIL");
	#endif /* DEBUG_0 */

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
		g_test_procs[i].m_stack_size=0x200;
	}

	g_test_procs[0].mpf_start_pc = &send_message_test;
	g_test_procs[1].mpf_start_pc = &receive_message_test;
	g_test_procs[2].mpf_start_pc = &memory_management_test;
	g_test_procs[3].mpf_start_pc = &priority_test;
	g_test_procs[4].mpf_start_pc = &preemption_check;
	g_test_procs[5].mpf_start_pc = &blocked_resource_test;

	set_up_testing_statements();
}

/**
 * @brief: tests send, send_delay and receive message (blocking)
 * is blocked on receive for 10 seconds, receives a delayed message
 * then remains blocked on receive indefinitely 
 */
void send_message_test(void)
{
	MSG_BUF *msg = NULL;
	int sender_id = -1;

	//send delayed message (to be used in testing with proccess 6)
	msg = (MSG_BUF *)request_memory_block();
	delayed_send(PID_P1, msg, PROC_1_DELAY);

	//send message to process 2 (for process 2 receive message test)
	msg = (MSG_BUF *)request_memory_block();
	msg->mtype = DEFAULT;
	msg->mtext[0] = 'h';
	msg->mtext[1] = 'i';
	msg->mtext[2] = '\0';
	send_message(PID_P2, msg);

	//type in commands AFTER test 4 is done
	msg = (MSG_BUF *)request_memory_block();
	msg->mtype = KCD_REG;
	msg->mtext[0] = '%';
	msg->mtext[1] = 'R';
	msg->mtext[2] = '\0';
	send_message(PID_KCD, msg);

	while (TRUE) {
		msg = (MSG_BUF *)receive_message(&sender_id);
		if (sender_id == PID_P1) {
			process_6_blocked = FALSE;
		} else if (sender_id == PID_KCD && msg->mtext[0] == '%' && msg->mtext[1] == 'R') {
			msg->mtext[0] = '\r';
			msg->mtext[1] = '1';
			msg->mtext[2] = ' ';
			msg->mtext[3] = 'o';
			msg->mtext[4] = 'k';
			msg->mtext[5] = '\r';
			msg->mtext[6] = '\0';
			msg->mtype = DEFAULT;
			send_message(PID_CRT, msg);
		}

		release_memory_block(msg);
	}
}

/**
 * @brief: Process 2: Tests received message
 * This processes receives a message without being blocked, then
 * is blocked indefinitely on receive
 */
void receive_message_test(void)
{
	MSG_BUF *msg = NULL;
	int sender_id = -1, passed = TRUE;

	while (TRUE) {
		msg = (MSG_BUF *)receive_message(&sender_id);
		if (msg->mtext[0] != 'h' || msg->mtext[1] != 'i' || msg->mtext[2] != '\0' || sender_id != PID_P1) {
			passed = FALSE;
		}
		release_memory_block(msg);

        printTestResults(passed);
	}
}

/**
 * @brief: Process 3: check if memory management works
 */
void memory_management_test(void)
{
	int ran = FALSE;
	void *mem_blks[3];
 	int num_blks = 3;
 	int passed = TRUE;
	int i, j;

	while (TRUE) {

		if (!ran) {
			ran = TRUE;

			//	allocate some blocks of memory
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
		}

	 	release_processor();
	 }
}

/**
 * @brief: Process 4: check if handling of processes works (get and set priority queues)
 */
void priority_test(void)
{
	int ran = FALSE;
	int passed = TRUE;
	testing_priority = TRUE;

	while (TRUE) {
		if (!ran) {
			ran = TRUE;

			//check to see if we can change null process priority
			if (set_process_priority(0, HIGH) == RTX_OK) {
				passed = FALSE;
			}

			//make sure it actually didn't end up changed the null proc priority
			if (get_process_priority(0) != LOWEST + 1) {
				passed = FALSE;
			}

			//set this processes' priority to the same one
			if (set_process_priority(PID_P4, MEDIUM) == RTX_ERR) {
				passed = FALSE;
			}

			//set this processes' priority to a higher one
			if (set_process_priority(PID_P4, HIGH) == RTX_ERR) {
				passed = FALSE;
			}

			//go back to medium
			if (set_process_priority(PID_P4, MEDIUM) == RTX_ERR) {
				passed = FALSE;
			}

			//set non-current process priority lower than current
			if (set_process_priority(PID_P5, LOW) == RTX_ERR) {
				passed = FALSE;
			}

			if (!passed) {
				testing_priority = FALSE;
				release_processor();
			}

			//set another processes priority to higher than this one
			process4_preempted = TRUE;
			if (set_process_priority(PID_P5, HIGH) == RTX_ERR) {
				testing_priority = FALSE;
				release_processor();
			}
		}

		release_processor();
	}
}

/**
 * @brief: Process 5: check if pre-emption occured with pid 4
 */
void preemption_check(void)
{
	int passed = TRUE, ran = FALSE;

	while (TRUE) {
		if (!ran) {
			ran = TRUE;

			if (set_process_priority(PID_P5, MEDIUM) == RTX_ERR) {
				passed = FALSE;
			}

			if (testing_priority) {
				testing_priority = FALSE;

				if (process4_preempted) {
					process4_preempted = FALSE;
				} else {
					passed = FALSE; //we pre-empted when we weren't supposed to
				}

			} else {
				passed = FALSE;
			}

			printTestResults(passed);
		}
		
		release_processor();
	}
}

/**
 * @brief: Process 6: Allocates all blocks of memory
 *	blocks on resource state for ~10 seconds
 * until process 1 unblocks
*/
void blocked_resource_test(void)
{
	int i = 0, ran = FALSE, unblocked = FALSE;

	while (TRUE) {

		if (!ran) {
			i = 0;
			ran = TRUE;
			process_6_blocked = TRUE;

			while (process_6_blocked) {
				mem_blks[i++] = request_memory_block();
			}

			unblocked = TRUE;

			while (i >= 0) {
				release_memory_block(mem_blks[--i]);
			}

			printTestResults(unblocked);
		}

		release_processor();
	}
}

