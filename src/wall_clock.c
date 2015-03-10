#include "wall_clock.h"
#include "timer.h"
#include "printf.h"

#define FALSE 0
#define TRUE 1
#define UART_INPUT 2
#define CLOCK_REG 3
#define BUFF_SIZE 100

void wall_clock_proc(void)
{
	MSG_BUF *msg;
	int hr, min, sec;
	int sender_id = -1;
	int clock_count = 0;
	int clock_is_on = TRUE;
	int increment = RTX_OK;

	msg = (MSG_BUF *) request_memory_block();
	msg->mtype = KCD_REG;
	msg->mtext[0] = '%';
	msg->mtext[1] = 'W';
	msg->mtext[2] = '\0';
	send_message(PID_KCD, (void *)msg);
	
	msg = (MSG_BUF *) request_memory_block();
	msg->mtype = WALL_CLOCK;
	increment = delayed_send(PID_CLOCK, msg, SECOND);

	if (increment == RTX_ERR) release_memory_block(msg);

	while (TRUE) {
		msg = (MSG_BUF *) receive_message(&sender_id);

		if (msg->mtype == WALL_CLOCK) {

			if (clock_is_on) display_time(++clock_count);
			increment = delayed_send(PID_CLOCK, msg, SECOND);
		} else if (msg->mtext[0] == '%' && msg->mtext[1] == 'W') {

			switch (msg->mtext[2]) {
				case 'R':
					clock_count = 0;
					if (increment == RTX_ERR) {
						msg->mtype = WALL_CLOCK;
						increment = delayed_send(PID_CLOCK, msg, SECOND);
					} else {
						release_memory_block(msg);
					}

					clock_is_on = TRUE;
					display_time(clock_count);
				break;

				case 'S':
					// %WS hh:mm:ss
					if ((msg->mtext[3] == ' ') && (msg->mtext[6] == ':') && (msg->mtext[9] == ':')) {
						hr = to_int(msg->mtext[4]) * 10; //first hour digit
						hr += to_int(msg->mtext[5]); //second hour digit
						min = to_int(msg->mtext[7]) * 10; //first minute digit
						min += to_int(msg->mtext[8]); //second minute digit
						sec = to_int(msg->mtext[10]) * 10; //first second digit
						sec += to_int(msg->mtext[11]); //second second digit
					
						if (hr <= 23 && min <= 60 && sec <= 60 && hr >= 0 && min >= 0 && sec >= 0) {
							clock_count = (sec + (min + hr * 60) * 60);
							if (increment == RTX_ERR) {
								msg->mtype = WALL_CLOCK;
								increment = delayed_send(PID_CLOCK, msg, SECOND);
							} else {
								release_memory_block(msg);
							}
							clock_is_on = TRUE;
							display_time(clock_count);
						}	
					}
				break;

				case 'T':
					clock_count = 0;
					clock_is_on = FALSE;
					release_memory_block(msg);
				break;
			}
		}

		if (increment == RTX_ERR) {
			release_memory_block(msg);
		}
	}
}

int to_int (uint8_t num) {
    if (num <= '9' && num >= '0') {
        return num -'0';
    }
    return RTX_ERR;
}

void display_time(int curr_time) {
	int s,h,m;
	MSG_BUF *display_msg = (MSG_BUF *)request_memory_block();

	curr_time = curr_time % 86400;
	s = curr_time % 60;
	h = curr_time / 3600;
	m = (curr_time / 60) % 60;

	display_msg->mtype = DEFAULT;

	display_msg->mtext[0] = h/10 + '0';
	display_msg->mtext[1] = h % 10 + '0';
	display_msg->mtext[2] = ':';
	display_msg->mtext[3] = m/10 + '0';
	display_msg->mtext[4] = m % 10 + '0';
	display_msg->mtext[5] = ':';
	display_msg->mtext[6] = s/10 + '0';
	display_msg->mtext[7] = s % 10 + '0';
	display_msg->mtext[8] = '\r';
	display_msg->mtext[9] = '\0';
	
	send_message(PID_CRT, display_msg);
}
