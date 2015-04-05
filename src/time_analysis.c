#include "rtx.h"
#include "printf.h"

extern int start_timer(void);
extern int pause_timer(void);
extern void stop_timer(void);

/**
 * Timer version of request_memory_block, for timing analysis
 */
void *request_memory_block_timed(void) {
	void *blk = NULL;
	int timer = start_timer();

	blk = _request_memory_block((U32)k_request_memory_block);

	if (timer) {
		timer = pause_timer();
		printf("request_memory_block took: %d (* 0.48) us \r\n", timer);
		stop_timer();
	}

	return blk;
}

/**
 * Timer version of send_message, for timing analysis
 */
int send_message_timed(int pid, void *p_msg) {
	int send_status = -1;
	int timer = start_timer();

	send_status = _send_message((U32)k_send_message, pid, p_msg);

	if (timer) {
		timer = pause_timer();
		printf("send_message took: %d (* 0.48) us \r\n", timer);
		stop_timer();
	}

	return send_status;
}

/**
 * Timer version of receive_message, for timing analysis
 */
void *receive_message_timed(int *p_pid) {
	void *msg = NULL;
	int timer = start_timer();

	msg = _receive_message((U32)k_receive_message, p_pid);

	if (timer) {
		timer = pause_timer();
		printf("receive_message took: %d (* 0.48) us \r\n", timer);
		stop_timer();
	}

	return msg;
}
