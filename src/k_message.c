#include "k_message.h"

/*
int send_message (uint32 receiving_pid, msg_t *env){
	atomic(on);
	sender_procid = current pid
	destination_procid = receiving_pid;
	PCB *receiving_proc = get_pcb_from_pid(receiving_pid);
	enqueue env onto the msg_queue of receiving_proc;
	if(receiving_proc->state is BLOCKED_ON_RECEIVE){
		set receiving_proc state to ready;
		rpq_enqueue(receiving_proc);
	}
	atomic(off);
}*/

/*
int delayed_send(uint32 receiving_pid, msg_t *env, int delay){
	same as send_message except after delay
}


// blocking
void *receive_message( int *sender_id)
{
	atomic(on);
	while (current_process msg_queue is empty) {
		set current_process state to BLOCKED_ON_RECEIVE;
		release_processor();
	}
	msg_t *env = dequeue current_process msg queue;
	atomic(off);
	return env;
}

// non-blocking
void *receive_message( int *sender_id)
{
	atomic(on);
	while (current_process msg_queue is empty) {
		set current_process state to BLOCKED_ON_RECEIVE;
		release_processor();
	}
	msg_t *env = dequeue current_process msg queue;
	atomic(off);
	return env;
}*/
