/**
 * @file:   k_process.h
 * @brief:  process management hearder file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: Assuming there are only two user processes in the system
 */

#ifndef K_MESSAGE_H_
#define K_MESSAGE_H_

#include "k_rtx.h"
#include "k_queue.h"

typedef struct msg_t
{
	int mtype;              /* user defined message type */
	char mtext[1];          /* body of the message */
	struct msg_t *next;
	U32 s_pid;		/* sender id */
	U32 d_pid;		/* destination id */
} msg_t;


//int send_message (uint32 receiving_pid, msg_t *env);
/*
int delayed_send(uint32 receiving_pid, msg_t *env, int delay);

// blocking
void *receive_message( int *sender_id);

// non-blocking
void *receive_message( int *sender_id);*/

#endif /* ! K_MESSAGE_H_ */
