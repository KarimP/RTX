#include "irq.h"
#include "k_rtx.h"
#include "k_message.h"
#include "k_process.h"
#include "k_memory.h"

extern void atomic(int);
extern queue *delayed_queue;

void timer_irq_proc(void)
{
	msg_Node *msg = (msg_Node *)delayed_queue->first;

    while (msg != NULL) {
        if (--msg->expire == 0) {
            dequeue(delayed_queue);
            k_send_message_with_node(msg);
            msg = (msg_Node *)delayed_queue->first;
        } else {
            msg = msg->next;
        }
    }
}

void uart_irq_proc(char key)
{
	int sender_id = -1;
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
	MSG_BUF *msg = NULL;
	char *buf = '\0';
	int forward_to_kcd = TRUE;

	#ifdef DEBUG_1
	printf("\n uart iprocess running \n");
	#endif /* DEBUG_1 */

	#ifdef DEBUG_HOTKEYS
	switch (key) { // printing process list and memory blocks available still left
		case '!':
			print_ready_procs();
			forward_to_kcd = FALSE;
			break;
		// case '"':
		case '@':
			print_mem_blocked_procs();
			forward_to_kcd = FALSE;
			break;
		// case '/':
		case '#':
			print_receive_blocked_procs();
			forward_to_kcd = FALSE;
			break;
		case '$':
			print_number_of_memory_blocks();
			forward_to_kcd = FALSE;
			break;
		// case '?':
		case '^':
			print_list_of_processes();
			forward_to_kcd = FALSE;
			break;
	}
	#endif

	if (forward_to_kcd && key) {
		msg = (MSG_BUF *) k_non_blocking_request_memory_block();
		if (msg) {
			msg->mtype = UART_INPUT;
			msg->mtext[0] = key;
			msg->mtext[1] = '\0';

			k_non_blocking_send_message(PID_KCD, msg);
		}
	}

	msg = (MSG_BUF *) k_non_blocking_receive_message(&sender_id);

	while (msg != NULL) {
		buf = msg->mtext;

		while (*buf != '\0') {
			pUart->THR = *(buf);

			if (*(buf++) == '\r') {
				pUart->THR = '\n';
			}
		}
		k_non_blocking_release_memory_block(msg);

		msg = (MSG_BUF *) k_non_blocking_receive_message(&sender_id);
	}
}
