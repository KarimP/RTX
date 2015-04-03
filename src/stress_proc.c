#include "stress_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/**
 * @brief: Stress test A: Tests recieving commands
 * and then continuously sends messages to stress test B 
 * if there is still memory
*/
void proc_A(void)
{
    MSG_BUF *msg = NULL;
    int num = 0;
    int sender_id = -1;
    
    msg = (MSG_BUF *)request_memory_block();
    msg->mtype = KCD_REG;
    msg->mtext[0] = '%';
    msg->mtext[1] = 'Z';
    msg->mtext[2] = '\0';
    send_message(PID_KCD, msg);
    
    while (TRUE) {
        msg = (MSG_BUF *)receive_message(&sender_id);
        if (sender_id == PID_KCD && msg->mtext[0] == '%' && msg->mtext[1] == 'Z') {
            release_memory_block(msg);
            break;
        }else{
            release_memory_block(msg);
        }

        release_memory_block(msg);
    }
    
    while (TRUE) {
        msg = (MSG_BUF *)request_memory_block();
        msg->mtype = COUNT_REPORT;
        msg->mtext[0] = num;
        msg->mtext[1] = '\0';
        send_message(PID_B, msg);

        #ifdef DEBUG_1
        printf("%d \r\n", num);
        #endif /* DEBUG_1 */
        
        num = num + 1;
        release_processor();
    }
}

/**
 * @brief: Stress test B: Forwards messages on to stress test C
*/
void proc_B(void)
{
    MSG_BUF *msg = NULL;
    int sender_id = -1;
    while (TRUE) {
        msg = (MSG_BUF *)receive_message(&sender_id);
        send_message(PID_C, msg);
    }
}

/**
 * @brief: Stress test C: Enqueues messages onto local message 
 * queue while hibernating for 10 seconds and processes the
 * messages.
*/
void proc_C(void)
{
    MSG_BUF *p;
    MSG_BUF *q;

    msg_Node* msg = NULL;
    int sender_id = -1;
    
    // Local message queue
    queue local_msg_queue;
    initialize_queue(&local_msg_queue);

    while(TRUE)
    {
        if (isEmpty(&local_msg_queue))
        {
            p = (MSG_BUF*) receive_message(&sender_id);
        }
        else {
            msg = (msg_Node *)local_msg_queue.first;
            dequeue(&local_msg_queue);
            p = (MSG_BUF *) msg->msgbuf;
        }
        
        if (p->mtype == COUNT_REPORT)
        {
            if (p->mtext[0] % 20 == 0)
            {   
                p->mtext[0] = 'P';
                p->mtext[1] = 'r';
                p->mtext[2] = 'o';
                p->mtext[3] = 'c';
                p->mtext[4] = 'e';
                p->mtext[5] = 's';
                p->mtext[6] = 's';
                p->mtext[7] = ' ';
                p->mtext[8] = 'C';
                p->mtext[9] = '\r';
                p->mtext[10] = '\0';
                p->mtype = DEFAULT;

                send_message(PID_CRT, p);
                
                //Hibernate for 10 seconds
                q = (MSG_BUF *)request_memory_block();
                q->mtype = HIBERNATE;
                
                delayed_send(PID_C, q, DELAY10);
                while(TRUE) {
                    p = (MSG_BUF*) receive_message(&sender_id);
                    if (p->mtype == HIBERNATE)
                    {
                        break;
                    }
                    else {
                        enqueue(&local_msg_queue, (queue_node *)p);
                    }
                }
            }
        }

        release_memory_block(p); 
        release_processor();
    }
}
