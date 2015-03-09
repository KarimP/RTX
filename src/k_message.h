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

typedef struct msg_Node
{
	struct msg_Node *next;
	U32 s_pid;		/* sender id */
	U32 d_pid;		/* destination id */
    int expire_time;
	void *msgbuf;
} msg_Node;


//int send_message (uint32 receiving_pid, msg_t *env);
/*
int delayed_send(uint32 receiving_pid, msg_t *env, int delay);

// blocking
void *receive_message( int *sender_id);

// non-blocking
void *receive_message( int *sender_id);*/

#endif /* ! K_MESSAGE_H_ */
