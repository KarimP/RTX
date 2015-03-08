#include "irq.h"
#include "k_rtx.h"
#include "printf.h"
#include "k_message.h"
#include "k_process.h"
#include "k_memory.h"

#define BUFF_SIZE 100
uint8_t input_buffer[BUFF_SIZE] = "";
int index = 0;

void k_irq_handler(irq_type type){
	switch(type){
		case TIMER:
					//need to perform a context switch to the timer Iprocess
					break;
		case UART:
					//need to perform a context switch to the uart Iprocess
					break;
		default:
				break;
	}

	//k_release_processor();
}

void uart_irq_proc(char key)
{
	MSG_BUF * msg;
	printf("\n uart iprocess running \n");

	// atomic(ON);
	#ifdef DEBUG_HOTKEYS
		switch (key) { // printing process list and memory blocks available still left
			case '\r':
				enter = 1;
				break;
			case '!':
				print_ready_procs();
				break;
			case '@':
				print_mem_blocked_procs();
				break;
			case '#':
				print_receive_blocked_procs();
				break;
			default:
				break;
		}
	#endif

	msg = (MSG_BUF *) k_request_memory_block();
	msg->mtype = UART_INPUT;
	msg->mtext[0] = key;
	msg->mtext[1] = '\0';

	k_send_message(PID_KCD, msg);
}
