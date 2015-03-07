#include "k_irq.h"
#include "k_process.h"

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
	
	k_release_processor();
}
