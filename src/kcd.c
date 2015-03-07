#include "kcd.h"
#include "printf.h"

#define FALSE 0
#define TRUE 1
#define UART_INPUT 2
#define CRT_PID 8

char *buf;
int buf_size;

int sender_id;

char *commands;
int *command_procs;
int num_commands;

int i;
int foundCommand;

void kcd_proc()
{
	MSG_BUF *msg = NULL;
    MSG_BUF *command_msg = NULL;
    buf = (char *)request_memory_block();
    buf_size = 0;

    sender_id = -1;

    commands = (char *)request_memory_block();
    command_procs = (int *)request_memory_block();
    num_commands = 0;

    i = 0;
    foundCommand = FALSE;
    while(TRUE) {
        // k_release_processor();
        msg = (MSG_BUF *)receive_message(&sender_id);

        if (msg != NULL) {
            switch (msg->mtype) {
                case UART_INPUT:
                {
                    if (msg->mtext[0] == '%' && buf_size == 0) {
                        foundCommand = TRUE;
                    }

                    if (foundCommand && buf_size == 1) {
                        foundCommand = FALSE;
                        for (i = 0; i < num_commands; i++) {
                            if(commands[i] == msg->mtext[0]) {
                                foundCommand = TRUE;
                                sender_id = command_procs[i];
                                break;
                            }
                        }
                    }

                    if (foundCommand) {
                        buf[buf_size++] = msg->mtext[0];
                    }

                    if (foundCommand && (msg->mtext[0] == '\n' || msg->mtext[0] == '%') && buf_size > 1) {

                        buf[buf_size++] = '\0';
                        command_msg = (MSG_BUF *)request_memory_block();
                        command_msg->mtype = DEFAULT;

                        for (i = 0; i < buf_size; ++i) {
                            command_msg->mtext[i] = buf[i];
                        }

                        send_message(sender_id, (void *)command_msg);

                        buf_size = 0;
                        if(msg->mtext[0] == '%') {
                            buf[buf_size++] = msg->mtext[0];
                        }
                    }

                    if (buf_size > MEM_BLK_SIZE) buf_size = 0;

                    msg->mtype = DEFAULT;
                    send_message(CRT_PID, (void *)msg);
                    break;
                }
                case KCD_REG:
                    if (msg->mtext[0] == '%') {
                        commands[num_commands] = msg->mtext[1];
                        command_procs[num_commands++] = sender_id;
                    }
                    release_memory_block((void *)msg);
                    break;
            }
        }
    }
}

// ************************ Alternate METHOD with UART sending entire line in message ************************* //
// char *commands;
// int *command_procs;
// int num_commands;
// int sender_id;
// int i;

// void kcd_proc()
// {
//     MSG_BUF *msg = NULL;
//     sender_id = -1;
//     i = 0;

//     while(TRUE) {
//         // k_release_processor();
//         msg = (MSG_BUF *)receive_message(&sender_id);
//         if (msg != NULL) {
//             switch (msg->mtype) {
//                 case UART_INPUT:
//                 {
//                     if (msg->mtext[0] == '%') {
//                         for (i = 0; i < num_commands; i++) {
//                             if(commands[i] == msg->mtext[0]) {
//                                 msg->mtype = KCD_RETURN;
//                                 send_message(command_procs[i], (void *)msg);
//                                 break;
//                             }
//                         }
//                     }
//                     msg->mtype = DEFAULT;
//                     send_message(CRT_PID, (void *)msg);
//                     break;
//                 }
//                 case KCD_REG:
//                     if (msg->mtext[0] == '%') {
//                         commands[num_commands] = msg->mtext[1];
//                         command_procs[num_commands++] = sender_id;
//                     }
//                     release_memory_block((void *)msg);
//                     break;
//                 default:
//                     release_memory_block((void *)msg);
//             }
//         }
//     }
// }
