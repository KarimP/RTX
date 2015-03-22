
#ifndef K_MESSAGE_H_
#define K_MESSAGE_H_

//#include "k_rtx.h"
#include "k_queue.h"

/* Kernel message envelope  */
typedef struct msg_Node
{
	struct msg_Node *next;
	U32 s_pid;		/* sender id */
	U32 d_pid;		/* destination id */
    int expire;		/* time left */
	void *msgbuf;	/* User message envelope */
} msg_Node;

#endif /* ! K_MESSAGE_H_ */
