/**
 * @brief timer.c - Timer example code. Tiemr IRQ is invoked every 1ms
 * @author T. Reidemeister
 * @author Y. Huang
 * @author NXP Semiconductors
 * @date 2012/02/12
 */

#include <LPC17xx.h>
#include "timer.h"
#include "irq.h"
#include "k_process.h"
#include "k_rtx.h"
#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

#define BIT(X) (1<<X)

extern int receiving_proc_unblock;
extern PCB **gp_pcbs;
extern PROC_INIT g_proc_table[NUM_PROCS];
extern PCB *gp_current_process;

volatile uint32_t g_timer = 0; //counter used by timer 1 for timing analysis
TIMER_STATE timer1_state = STOPPED; //state of counter used by timer 1
int timer1_paused_pid = -1; //current process that is using timer 1

//processors to ignore for timing analysis
int timer1_ignore_pids[1] = {0};
int num_timer1_ignore_pids = 1;

extern void atomic(int);

/**
 * @brief: initialize timer. Only timer 0 is supported
 */
uint32_t timer_init(uint8_t n_timer)
{
	LPC_TIM_TypeDef *pTimer;
	if (n_timer == 0) {
		/*
		Steps 1 & 2: system control configuration.
		Under CMSIS, system_LPC17xx.c does these two steps

		-----------------------------------------------------
		Step 1: Power control configuration.
		        See table 46 pg63 in LPC17xx_UM
		-----------------------------------------------------
		Enable UART0 power, this is the default setting
		done in system_LPC17xx.c under CMSIS.
		Enclose the code for your refrence
		//LPC_SC->PCONP |= BIT(1);

		-----------------------------------------------------
		Step2: Select the clock source,
		       default PCLK=CCLK/4 , where CCLK = 100MHZ.
		       See tables 40 & 42 on pg56-57 in LPC17xx_UM.
		-----------------------------------------------------
		Check the PLL0 configuration to see how XTAL=12.0MHZ
		gets to CCLK=100MHZ in system_LPC17xx.c file.
		PCLK = CCLK/4, default setting in system_LPC17xx.c.
		Enclose the code for your reference
		//LPC_SC->PCLKSEL0 &= ~(BIT(3)|BIT(2));

		-----------------------------------------------------
		Step 3: Pin Ctrl Block configuration.
		        Optional, not used in this example
		        See Table 82 on pg110 in LPC17xx_UM
		-----------------------------------------------------
		*/
		pTimer = (LPC_TIM_TypeDef *) LPC_TIM0;

		/*
		-----------------------------------------------------
		Step 4: Interrupts configuration
		-----------------------------------------------------
		*/

		/* Step 4.1: Prescale Register PR setting
		   CCLK = 100 MHZ, PCLK = CCLK/4 = 25 MHZ
		   2*(12499 + 1)*(1/25) * 10^(-6) s = 10^(-3) s = 1 ms
		   TC (Timer Counter) toggles b/w 0 and 1 every 12500 PCLKs
		   see MR setting below
		*/
		pTimer->PR = 12499;
	   	// pTimer->PR = 124;

		/* Step 4.2: MR setting, see section 21.6.7 on pg496 of LPC17xx_UM. */
		pTimer->MR0 = 1;

		/* Step 4.3: MCR setting, see table 429 on pg496 of LPC17xx_UM.
		   Interrupt on MR0: when MR0 mathches the value in the TC,
		                     generate an interrupt.
		   Reset on MR0: Reset TC if MR0 mathches it.
		*/
		pTimer->MCR = BIT(0) | BIT(1);

		g_timer = 0;

		/* Step 4.4: CSMSIS enable timer0 IRQ */
		NVIC_EnableIRQ(TIMER0_IRQn);

		/* Step 4.5: Enable the TCR. See table 427 on pg494 of LPC17xx_UM. */
		pTimer->TCR = 1;

	} else if (n_timer == 1) {

		/*
		Steps 1 & 2: system control configuration.
		Under CMSIS, system_LPC17xx.c does these two steps

		-----------------------------------------------------
		Step 1: Power control configuration.
		        See table 46 pg63 in LPC17xx_UM
		-----------------------------------------------------
		Enable UART0 power, this is the default setting
		done in system_LPC17xx.c under CMSIS.
		Enclose the code for your refrence
		//LPC_SC->PCONP |= BIT(1);

		-----------------------------------------------------
		Step2: Select the clock source,
		       default PCLK=CCLK/4 , where CCLK = 100MHZ.
		       See tables 40 & 42 on pg56-57 in LPC17xx_UM.
		-----------------------------------------------------
		Check the PLL0 configuration to see how XTAL=12.0MHZ
		gets to CCLK=100MHZ in system_LPC17xx.c file.
		PCLK = CCLK/4, default setting in system_LPC17xx.c.
		Enclose the code for your reference
		//LPC_SC->PCLKSEL0 &= ~(BIT(3)|BIT(2));

		-----------------------------------------------------
		Step 3: Pin Ctrl Block configuration.
		        Optional, not used in this example
		        See Table 82 on pg110 in LPC17xx_UM
		-----------------------------------------------------
		*/
		pTimer = (LPC_TIM_TypeDef *) LPC_TIM1;

		/*
		-----------------------------------------------------
		Step 4: Interrupts configuration
		-----------------------------------------------------
		*/

		/* Step 4.1: Prescale Register PR setting
		   CCLK = 100 MHZ, PCLK = CCLK/4 = 25 MHZ
		   2*(12499 + 1)*(1/25) * 10^(-6) s = 10^(-3) s = 1 ms
		   TC (Timer Counter) toggles b/w 0 and 1 every 12500 PCLKs
		   see MR setting below
		   
		   12499 - 1s
		   124 - 10us
		   5 - 0.48us
		*/
		pTimer->PR = 5;

		/* Step 4.2: MR setting, see section 21.6.7 on pg496 of LPC17xx_UM. */
		pTimer->MR0 = 1;

		/* Step 4.3: MCR setting, see table 429 on pg496 of LPC17xx_UM.
		   Interrupt on MR0: when MR0 mathches the value in the TC,
		                     generate an interrupt.
		   Reset on MR0: Reset TC if MR0 mathches it.
		*/
		pTimer->MCR = BIT(0) | BIT(1);

		g_timer = 0;

		/* Step 4.4: CSMSIS enable timer1 IRQ */
		NVIC_EnableIRQ(TIMER1_IRQn);

		/* Step 4.5: Enable the TCR. See table 427 on pg494 of LPC17xx_UM. */
		pTimer->TCR = 1;

		//////////////////////////////////////////////////////////
		// pTimer->PR = 124;
		// pTimer->TCR = 0x02; // counter reset, table 427 on pg494 of LPC17xx_UM
		// pTimer->TCR = 0x01; // counter enable, table 427 on pg494 of LPC17xx_UM
		// pTimer->MCR &= ~BIT(0);
		// pTimer->MCR &= ~BIT(1);
		// pTimer->MCR = BIT(0) | BIT(1);

		// NVIC_EnableIRQ(TIMER1_IRQn);
		// return 1;
	}

	return 0;
}

/**
 * @brief: use CMSIS ISR for TIMER0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine.
 *       The actual c_TIMER0_IRQHandler does the rest of irq handling
 */
__asm void TIMER0_IRQHandler(void)
{
	PRESERVE8
	IMPORT c_TIMER0_IRQHandler
	PUSH{r4-r11, lr}
	BL c_TIMER0_IRQHandler
	POP{r4-r11, pc}
}
/**
 * @brief: c TIMER0 IRQ Handler
 */
void c_TIMER0_IRQHandler(void)
{
	PCB *current_process = gp_current_process;

	atomic(ON);

	LPC_TIM0->IR = BIT(0);   // ack inttrupt, see section  21.6.1 on pg 493 of LPC17XX_UM

	//set current process to i-process
	gp_current_process->m_state = RDY;
	gp_current_process = gp_pcbs[INDEX_TIMER_IPROC];
	gp_current_process->m_state = RUN;

	receiving_proc_unblock = FALSE;
	timer_irq_proc();	// call timer i-proc

	//restore current process
	gp_current_process->m_state = WAITING_FOR_INTERRUPT;
	gp_current_process = current_process;
	gp_current_process->m_state = RUN;

	atomic(OFF);

	if (receiving_proc_unblock) {
		k_release_processor();
	}
}

/**
 * @brief: use CMSIS ISR for TIMER1 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine.
 *       The actual c_TIMER1_IRQHandler does the rest of irq handling
 */
__asm void TIMER1_IRQHandler(void)
{
	PRESERVE8
	IMPORT c_TIMER1_IRQHandler
	PUSH{r4-r11, lr}
	BL c_TIMER1_IRQHandler
	POP{r4-r11, pc}
}
/**
 * @brief: c TIMER1 IRQ Handler
 */
void c_TIMER1_IRQHandler(void)
{
	LPC_TIM1->IR = BIT(0);   // ack inttrupt, see section  21.6.1 on pg 493 of LPC17XX_UM
	if (timer1_state == RUNNING) g_timer++;
}

/**
 * Attempt to start the timer counter if you're not in an ignore list
 * @return 1 if counter successfully started, 0 otherwise
 */
int start_timer (void) {
	int i;
	if (timer1_state == STOPPED) {
		if (num_timer1_ignore_pids) {
			for (i = 0; i < num_timer1_ignore_pids; ++i) {
				if (gp_current_process->m_pid == timer1_ignore_pids[i]) return FALSE;
			}
		}
		timer1_state = RUNNING;
		g_timer = 0;
		return TRUE;
	}

	return FALSE;
}

/**
 * Start up counter again after it's been paused
 */
void continue_timer(void) {
	if (timer1_state == PAUSED && timer1_paused_pid == gp_current_process->m_pid) 
		timer1_state = RUNNING;
}

/**
 * Pause the counter, essentially a lock for the counter while you read the value
 * @return  value of counter
 */
int pause_timer(void) {
	if (timer1_state == RUNNING) {
		timer1_paused_pid = gp_current_process->m_pid;
		timer1_state = PAUSED;
		return g_timer;	
	}

	return -1;
}

/**
 * Halt the counter so other methods can start it
 */
void stop_timer(void) {
	timer1_state = STOPPED;
}
