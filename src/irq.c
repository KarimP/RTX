#include "irq.h"
#include "rtx.h"
#include "printf.h"
//#include "k_message.h"
//#include "k_process.h"
//#include "k_memory.h"

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



void uart_irq_proc(uint8_t key)
{
	int i;
	void *block;
	MSG_BUF * msg;
	int pid;
	printf("uart iprocess running\n");
	while(TRUE) {
		release_processor();
	}
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

	block = (void *) non_blocking_request_memory_block();
	msg = (MSG_BUF *) block;
	if (block != NULL) {
		if (key == '\r') { // newline
			for (i = 0; i < BUFF_SIZE; i++) {
				msg->mtext[i] = '\0';
			}
			msg->mtype = KCD_REG;
			pid = PID_KCD ;
			for (i = 0; i < index; i++) {
				msg->mtext[i] = input_buffer[i];
			}
			send_message(pid, (void *)msg);
		} else {
			for (i = 0; i < BUFF_SIZE; i++) {
				msg->mtext[i] = '\0';
			}
			msg->mtype = CRT_REG;
			pid = PID_CRT;
			msg->mtext[0] = key;
			send_message(pid, (void *)msg);
		}
	} else {
		if (key == '\r') {
			index = 0;
			for (i = 0; i < BUFF_SIZE; i++) {
				input_buffer[i] = '\0';
			}
		} else {
			input_buffer[index % BUFF_SIZE] = key;
			index++;
		}
	}
	// atomic(OFF);
}
