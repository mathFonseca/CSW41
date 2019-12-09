/*============================================================================
 *                              Laboratório 5
 *              EK-TM4C1294XL + Educational BooterPack MKII 
 *---------------------------------------------------------------------------*
 *                    Prof. André Schneider de Oliveira
 *            Universidade Tecnológica Federal do Paraná (UTFPR)
 *===========================================================================
 * Autores das bibliotecas:
 * 		Allan Patrick de Souza - <allansouza@alunos.utfpr.edu.br>
 * 		Guilherme Jacichen     - <jacichen@alunos.utfpr.edu.br>
 * 		Jessica Isoton Sampaio - <jessicasampaio@alunos.utfpr.edu.br>
 * 		Mariana Carrião        - <mcarriao@alunos.utfpr.edu.br>
 *===========================================================================*/
#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include "grlib/grlib.h"

/*----------------------------------------------------------------------------
 * include libraries from drivers
 *----------------------------------------------------------------------------*/

#include "cfaf128x128x16.h"
#include "servo.h"
#include "buttons.h"
#include "NewUart.h"
#include <math.h>

// To print on the screen
tContext sContext;

//Mutex
osMutexId mutex_display;
osMutexDef(mutex_display);

// Thread IDs
osThreadId  thread_scheduler_id,
			thread_prime_number_gen_id, 
			thread_uart_id, 
			thread_claw_control_id, 
			thread_fibonacci_number_gen_id;

// Timer
void preempt_callback() {
	osSignalSet(thread_scheduler_id, 0x01);
}
osTimerDef(preempt, preempt_callback);
osTimerId preempt_id;

typedef enum {
	NOT_READY,
	READY,
	EXECUTING
} task_status;

typedef struct {
	osThreadId thread_id;
	int static_priority;
	int current_priority;
	uint64_t expected_duration;
	float deadline;
	uint8_t ready_counter;
	uint8_t ready_max;
	task_status status;
	uint32_t start_tick;
} task_t;

// Tasks
task_t task_prime_number, task_uart, task_claw, task_fibonacci;
task_t *tasks[4], *execution_q[4];

/*----------------------------------------------------------------------------
 *  Messages
 *---------------------------------------------------------------------------*/
typedef struct {
	char msg;
} uart_isr_msg;

osMailQDef(uart_mail_q,2,uart_isr_msg);
osMailQId (uart_mail_q_id);

typedef struct {
	int velocidade;
} vel_msg;

osMailQDef(vel_mail_q,2,vel_msg);
osMailQId (vel_mail_q_id);

typedef struct {
	task_t *task;
	task_status status;
} task_ready_msg;

osMailQDef(task_ready_q, 2, task_ready_msg);
osMailQId(task_ready_q_id);

/*----------------------------------------------------------------------------
 *  Utilities
 *---------------------------------------------------------------------------*/
static void intToString(int64_t value, char * pBuf, uint32_t len, uint32_t base, uint8_t zeros){
	static const char* pAscii = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	bool n = false;
	int pos = 0, d = 0;
	int64_t tmpValue = value;

	// the buffer must not be null and at least have a length of 2 to handle one
	// digit and null-terminator
	if (pBuf == NULL || len < 2)
			return;

	// a valid base cannot be less than 2 or larger than 36
	// a base value of 2 means binary representation. A value of 1 would mean only zeros
	// a base larger than 36 can only be used if a larger alphabet were used.
	if (base < 2 || base > 36)
			return;

	if (zeros > len)
		return;
	
	// negative value
	if (value < 0)
	{
			tmpValue = -tmpValue;
			value    = -value;
			pBuf[pos++] = '-';
			n = true;
	}

	// calculate the required length of the buffer
	do {
			pos++;
			tmpValue /= base;
	} while(tmpValue > 0);


	if (pos > len)
			// the len parameter is invalid.
			return;

	if(zeros > pos){
		pBuf[zeros] = '\0';
		do{
			pBuf[d++ + (n ? 1 : 0)] = pAscii[0]; 
		}
		while(zeros > d + pos);
	}
	else
		pBuf[pos] = '\0';

	pos += d;
	do {
			pBuf[--pos] = pAscii[value % base];
			value /= base;
	} while(value > 0);
}

static void floatToString(float value, char *pBuf, uint32_t len, uint32_t base, uint8_t zeros, uint8_t precision){
	static const char* pAscii = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	uint8_t start = 0xFF;
	if(len < 2)
		return;
	
	if (base < 2 || base > 36)
		return;
	
	if(zeros + precision + 1 > len)
		return;
	
	intToString((int64_t) value, pBuf, len, base, zeros);
	while(pBuf[++start] != '\0' && start < len); 

	if(start + precision + 1 > len)
		return;
	
	pBuf[start+precision+1] = '\0';
	
	if(value < 0)
		value = -value;
	pBuf[start++] = '.';
	while(precision-- > 0){
		value -= (uint32_t) value;
		value *= (float) base;
		pBuf[start++] = pAscii[(uint32_t) value];
	}
}

void sortTasks(task_t** sorting_tasks, uint8_t length) {
	uint8_t i, j;
	task_t* aux;
	
	for(j = 0; j < length; j++) {
		for(i = 1; i < length; i++) {
			if(length > 1) {
				if(sorting_tasks[i]->current_priority < sorting_tasks[i-1]->current_priority) {
					aux = sorting_tasks[i];
					sorting_tasks[i] = sorting_tasks[i - 1];
					sorting_tasks[i-1] = aux;
				}
			}
		}
	}
}

void initTasks() {
	// Initialize the tasks
	task_prime_number.thread_id = thread_prime_number_gen_id;
	task_prime_number.static_priority = -100;
	task_prime_number.current_priority = task_prime_number.static_priority;
	task_prime_number.deadline = 0.3;
	task_prime_number.ready_max = 2;
	task_prime_number.ready_counter = task_prime_number.ready_max;
	task_prime_number.status = NOT_READY;
	task_prime_number.expected_duration = 1834000;

	task_claw.thread_id = thread_claw_control_id;
	task_claw.static_priority = 10;
	task_claw.current_priority = task_claw.static_priority;
	task_claw.deadline = 0.7;
	task_claw.ready_max = 1;
	task_claw.ready_counter = task_claw.ready_max;
	task_claw.status = NOT_READY;
	task_claw.expected_duration = 120000;

	task_fibonacci.thread_id = thread_fibonacci_number_gen_id;
	task_fibonacci.static_priority = 0;
	task_fibonacci.current_priority = task_fibonacci.static_priority;
	task_fibonacci.deadline = 0.5;
	task_fibonacci.ready_max = 10;
	task_fibonacci.ready_counter = task_fibonacci.ready_max;
	task_fibonacci.status = NOT_READY;
	task_fibonacci.expected_duration = 1305707;

	task_uart.thread_id = thread_uart_id;
	task_uart.static_priority = -30;
	task_uart.current_priority = task_uart.static_priority;
	task_uart.deadline = 0.1;
	task_uart.expected_duration = 802000;

	tasks[0] = &task_prime_number;
	tasks[1] = &task_claw;
	tasks[2] = &task_fibonacci;
	tasks[3] = &task_uart;
}

/*----------------------------------------------------------------------------
 *    Initializations
 *---------------------------------------------------------------------------*/
void init_all(){
	cfaf128x128x16Init();
	button_init();
	servo_init();
}

void init_display_context(){
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrYellow);
	GrContextBackgroundSet(&sContext, ClrBlack);
}

/*----------------------------------------------------------------------------
 *    Callbacks
 *---------------------------------------------------------------------------*/
void UART0_Handler(void){
	char m;
	uart_isr_msg * message;
	while((UART0->FR & (1<<4)) != 0);
	m = UART0->DR;
	UART0	->	RIS |= (1<<4);
	message = (uart_isr_msg*)osMailAlloc(uart_mail_q_id,0);
	if(message){
		message	-> msg = m;
		osMailPut(uart_mail_q_id,message); 
	}
	tasks[3]->status = READY;
	tasks[3]->start_tick = osKernelSysTick();
	execution_q[3] = tasks[3];
}

/*----------------------------------------------------------------------------
 *      Threads
 *---------------------------------------------------------------------------*/
void thread_scheduler(void const *argument) {
	uint8_t i, entries = 0, ready_tasks = 0;
	uint32_t execution_ticks;
	char pbuf[10];
	osEvent evt;
	task_ready_msg *msg;
	GrStringDraw(&sContext, "Prioridade: ", -1, 0, (sContext.psFont->ui8Height+2)*0, true);
	GrStringDraw(&sContext, "T de Relax.: ", -1, 0, (sContext.psFont->ui8Height+2)*1, true);
	initTasks();

	while(1) {
		// Verifica se alguma task terminou de executar
		evt = osMailGet(task_ready_q_id, 0);
		if(evt.status == osEventMail) {
			msg = (task_ready_msg *) evt.value.p;
			if(msg->task->status == EXECUTING && msg->status == NOT_READY) {
				execution_ticks = osKernelSysTick() - msg->task->start_tick;
				
				intToString(msg->task->static_priority, pbuf,  10, 10, 4);
				GrStringDraw(&sContext, (char*)pbuf, -1, 65, (sContext.psFont->ui8Height+2)*0, true);
				intToString(msg->task->expected_duration - execution_ticks, pbuf,  10, 10, 4);
				GrStringDraw(&sContext, (char*)pbuf, -1, 75, (sContext.psFont->ui8Height+2)*1, true);

				if(execution_ticks > msg->task->expected_duration * (1 + msg->task->deadline) 
					&& msg->task->static_priority == -100) {
						GrStringDraw(&sContext, "Master fault", -1, 0, (sContext.psFont->ui8Height+2)*12, true);
						while(1) {}
				   }

				else if(execution_ticks > msg->task->expected_duration * (1 + msg->task->deadline) 
					&& msg->task->static_priority > -100) {
						// print secondary fault
						msg->task->current_priority -= 10;
						GrStringDraw(&sContext, "Secondary Fault P+", -1, 0, (sContext.psFont->ui8Height+2)*12, true);
				}

				else if(execution_ticks < (msg->task->expected_duration * (1 + msg->task->deadline)) / 2
					&& msg->task->static_priority > -100) {
						// print secondary fault
						msg->task->current_priority += 10;
						GrStringDraw(&sContext, "Secondary Fault P-", -1, 0, (sContext.psFont->ui8Height+2)*12, true);
				}
				else
					GrStringDraw(&sContext, "#####################", -1, 0, (sContext.psFont->ui8Height+2)*12, true);
			}

			msg->task->status = msg->status;
			msg->task->ready_counter = msg->task->ready_max;
			msg->task->current_priority = msg->task->static_priority;

			for(i = 0; i < ready_tasks; i++)
				execution_q[i] = execution_q[i + 1];
			ready_tasks--;
			osMailFree(task_ready_q_id, msg);
		}

		// A cada 100 ms...
		if(entries >= 100) {
			entries = 0;
			ready_tasks = 0;
			for(i = 0; i < 3; i++) {
				tasks[i]->ready_counter--;
				if(tasks[i]->ready_counter <= 0) {
					tasks[i]->status = READY;
					tasks[i]->start_tick = osKernelSysTick();
					execution_q[ready_tasks] = tasks[i];
					ready_tasks++;
				}
				
			}
			sortTasks(execution_q, ready_tasks);
		}

		if(execution_q[0]->status != EXECUTING && execution_q[0]->status == READY) {
			execution_q[0]->status = EXECUTING;
			osSignalSet(execution_q[0]->thread_id, 0x01);
		}

		entries++;
		osSignalWait(0x01, osWaitForever);
	}
}
osThreadDef(thread_scheduler, osPriorityNormal, 1, 0);

void thread_prime_number_gen(void const *argument) {
	uint64_t n = 2, i;
	bool dividerFound, primeFound;
	char pbuf[10];
	task_ready_msg *msg;
	
	GrStringDraw(&sContext, "P: ", -1, 0, (sContext.psFont->ui8Height+2)*10, true);
	while(1) {
		osSignalWait(0x01, osWaitForever);

		dividerFound = false;
		primeFound = false;
		while(!primeFound) {
			dividerFound = false;
			if(n % 2 == 0)
				dividerFound = true;
			for(i = 3; i < sqrt(n) && !dividerFound; i += 2) {
				if(n % i == 0)
					dividerFound = true;
			}

			if(!dividerFound) {
				primeFound = true;
				intToString(n, pbuf, 10, 10, 9);
				GrStringDraw(&sContext, pbuf, -1, 30, (sContext.psFont->ui8Height+2)*10, true);
			}
			n++;
		}

		msg = (task_ready_msg *) osMailAlloc(task_ready_q_id, osWaitForever);
		msg->task = tasks[0];
		msg->status = NOT_READY;
		osMailPut(task_ready_q_id, msg);
	}
}
osThreadDef(thread_prime_number_gen, osPriorityNormal, 1, 0);

void thread_uart(void const *argument) {
	osEvent evt;
	uart_isr_msg *message = 0;
	task_ready_msg *msg_ready = 0;
	uart_print_string("\033[2J");
	uart_print_string("\033[0;0H");
	uart_print_string("\n\rEscolha a velocidade do movimento:");
	uart_print_string("\n\r1- Rápido");
	uart_print_string("\n\r2- Médio");
	uart_print_string("\n\r3- Lento");
	
	while(1) {
		osSignalWait(0x01, osWaitForever);
		evt = osMailGet(uart_mail_q_id, 0);
		while(evt.status!=osEventMail)
			evt = osMailGet(uart_mail_q_id, 0);
		
		message = (uart_isr_msg*) evt.value.p;
		if(message->msg == '1' || message->msg == '2' || message->msg == '3')
		{
			vel_msg *msg;
			msg = (vel_msg*)osMailAlloc(vel_mail_q_id,0);
			if(msg){
				msg	-> velocidade = (int)message->msg-48;
				osMailPut(vel_mail_q_id,msg); 
			}
		}			
		osMailFree(uart_mail_q_id, message);

		msg_ready = (task_ready_msg *) osMailAlloc(task_ready_q_id, osWaitForever);
		msg_ready->task = tasks[3];
		msg_ready->status = NOT_READY;
		osMailPut(task_ready_q_id, msg_ready);
	}
}
osThreadDef(thread_uart, osPriorityNormal, 1, 0);

void thread_claw_control(void const *argument) {
	int acao = 0;
	osEvent evt;
	vel_msg *message = 0;
	task_ready_msg *msg = 0;

	int delay = 0;
	uint8_t vel = 1;
	while(1)
	{
		osSignalWait(0x01, osWaitForever);
		
		evt = osMailGet(vel_mail_q_id,0);
		if(evt.status == osEventMail){
			message = (vel_msg*) evt.value.p;			
			vel = message->velocidade;
			osMailFree(vel_mail_q_id, message);
		}
		
		switch(acao){
			//-------------------
			//pega obj pos 1
			//-------------------
			case 0: 
				delay = 0;
				servo_write2(0x0000);
				servo_write3(0x0000);
				acao++;
				break;
			case 1:
				delay++;
				break;
			case 2:
				delay = 0;
				servo_write1(0xA000);
				servo_write0(0xE000);
				acao++;
				break;		
			case 3:
				delay++;
				break;
			case 4:
				delay = 0;
				servo_write3(0xFFFF);
				acao++;
				break;
			case 5:
				delay++;
				break;
			//-------------
			//sobe
			//--------------
			case 6:
				delay = 0;
				servo_write0(0x3000);	
				acao++;
				break;
			case 7:
				delay++;
				break;
			case 8:
				delay = 0;
				servo_write1(0x3000);
				acao++;
				break;
			case 9:
				delay++;
				break;
			//--------------------
			//larga obj 1 na pos 3
			//--------------------
			case 10:
				delay = 0;
				servo_write2(0xF300);
				acao++;
				break;
			case 11:
				delay++;
				break;
			case 12:
				delay = 0;
				servo_write1(0xA000);
				acao++;
				break;
			case 13:
				delay++;
				break;
			case 14:
				delay = 0;
				servo_write0(0xE000);
				acao++;
				break;
			case 15:
				delay++;
				break;
			case 16:
				delay = 0;
				servo_write3(0x0000);	
				acao++;
				break;
			case 17:
				delay++;
				break;
			//-----------------
			//sobe
			//-----------------
			case 18:
				delay = 0;
				servo_write0(0x3000);
				acao++;
				break;
			case 19:
				delay++;
				break;
			case 20:
				delay = 0;
				servo_write1(0x3000);	
				acao++;
				break;
			case 21:
				delay++;
				break;
			//---------------
			//pega obj pos 2
			//--------------
			case 22:
				delay = 0;
				servo_write2(0x7FFF);
				servo_write3(0x0000);
				acao++;
				break;
			case 23:
				delay++;
				break;
			case 24:
				delay = 0;
				servo_write1(0xA000);
				acao++;
				break;
			case 25:
				delay++;
				break;
			case 26:
				delay = 0;
				servo_write0(0x8000);	
				acao++;
				break;
			case 27:
				delay++;
				break;
			case 28:
				delay = 0;
				servo_write3(0xFFFF);
				acao++;
				break;
			case 29:
				delay++;
				break;
			//-----------------
			//sobe
			//-----------------
			case 30:
				delay = 0;
				servo_write0(0x3000);
				acao++;
				break;
			case 31:
				delay++;
				break;
			case 32:
				delay = 0;
				servo_write1(0x3000);	
				acao++;
				break;
			case 33:
				delay++;
				break;
			//------------------
			//solta obj pos 1
			//------------------
			case 34:
				delay = 0;
				servo_write2(0x0000);
				acao++;
				break;
			case 35:
				delay++;
				break;
			case 36:
				delay = 0;
				servo_write1(0xA000);	
				servo_write0(0xE000);				
				acao++;
				break;
			case 37:
				delay++;
				break;
			case 38:
				delay = 0;
				servo_write3(0x0000);	
				acao++;
				break;
			case 39:
				delay++;
				break;
			//-----------------
			//sobe
			//-----------------
			case 40:
				delay = 0;
				servo_write0(0x3000);
				acao++;
				break;
			case 41:
				delay++;
				break;
			case 42:
				delay = 0;
				servo_write1(0x3000);	
				acao++;
				break;
			case 43:
				delay++;
				break;
			//-----------------
			//pega obj pos 3
			//-----------------
			case 44:
				delay = 0;
				servo_write2(0xFF00);
				servo_write3(0x0000);
				servo_write1(0xA000);		
				acao++;
				break;
			case 45:
				delay++;
				break;
			case 46:
				delay = 0;
				servo_write0(0xE000);					
				acao++;
				break;
			case 47:
				delay++;
				break;
			case 48:
				delay = 0;
				servo_write3(0xFFFF);
				acao++;
				break;
			case 49:
				delay++;
				break;
			//-----------------
			//sobe
			//-----------------
			case 50:
				delay = 0;
				servo_write0(0x3000);
				acao++;
				break;
			case 51:
				delay++;
				break;
			case 52:
				delay = 0;
				servo_write1(0x3000);	
				acao++;
				break;
			case 53:
				delay++;
				break;
			//-----------------
			//Solta objeto pos 2
			//-----------------
			case 54:
				delay = 0;
				servo_write2(0x7FFF);
				servo_write1(0xA000);			
				acao++;
				break;
			case 55:
				delay++;
				break;
			case 56:
				delay = 0;
				servo_write0(0x8000);						
				acao++;
				break;
			case 57:
				delay++;
				break;
			case 58:
				delay = 0;
				servo_write3(0x0000);
				acao++;
				break;
			case 59:
				delay++;
				break;
			//-----------------
			//sobe
			//-----------------
			case 60:
				delay = 0;
				servo_write0(0x3000);
				acao++;
				break;
			case 61:
				delay++;
				break;
			case 62:
				delay = 0;
				servo_write1(0x3000);	
				acao++;
				break;
			case 63:
				delay++;
				break;			
		}
		osDelay(1);
		if(delay >= 10*vel)
		{
			acao++;
			delay = 0;
		}
		if(acao == 64)
			acao = 0;

		msg = (task_ready_msg *) osMailAlloc(task_ready_q_id, osWaitForever);
		msg->task = tasks[1];
		msg->status = NOT_READY;
		osMailPut(task_ready_q_id, msg);
		

		//47360, 119068,47360,119068,119068

	}
}
osThreadDef(thread_claw_control, osPriorityNormal, 1, 0);

void thread_fibonacci_number_gen(void const *argument) {
	uint64_t n1 = 0, n2 = 1, n3;
	char pbuf[10];
	task_ready_msg *msg = 0;

	GrStringDraw(&sContext, "F: ", -1, 0, (sContext.psFont->ui8Height+2)*11, true);
	while(1) {
		osSignalWait(0x01, osWaitForever);

		n3 = n1 + n2;
		// Imprime na tela
		intToString(n3, pbuf, 10, 10, 9);
		GrStringDraw(&sContext, pbuf, -1, 30, (sContext.psFont->ui8Height+2)*11, true);

		n1 = n2;
		n2 = n3;
		
		if(n3 > 700000000) {
			n1 = 0;
			n2 = 1;
		}

		msg = (task_ready_msg *) osMailAlloc(task_ready_q_id, osWaitForever);
		msg->task = tasks[2];
		msg->status = NOT_READY;
		osMailPut(task_ready_q_id, msg);

	}
}
osThreadDef(thread_fibonacci_number_gen, osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/
int main (void) {
	// Initialize the kernel
	osKernelInitialize();
	
	//Initializing all peripherals
	init_all();
	init_display_context();
	init_uart();	
	
	//Create the mutex
	mutex_display = osMutexCreate(osMutex(mutex_display));

	// Create the mail queues
	uart_mail_q_id = osMailCreate(osMailQ(uart_mail_q), NULL);
	vel_mail_q_id = osMailCreate(osMailQ(vel_mail_q), NULL);
	task_ready_q_id = osMailCreate(osMailQ(task_ready_q), NULL);

	// Create the timers
	preempt_id = osTimerCreate(osTimer(preempt), osTimerPeriodic, (void*) 0);
	
	osDelay(1000);
	//altura
	servo_write0(0x7FFF);
	//articulação
	servo_write1(0x7000);
	//local
	servo_write2(0x7FFF);
	//fecha e abre
	servo_write3(0xFFFF);
	
	// Create the threads
	thread_scheduler_id            = osThreadCreate(osThread(thread_scheduler), NULL);
	thread_prime_number_gen_id     = osThreadCreate(osThread(thread_prime_number_gen), NULL);
	thread_uart_id 		             = osThreadCreate(osThread(thread_uart), NULL);
	thread_claw_control_id         = osThreadCreate(osThread(thread_claw_control), NULL);
	thread_fibonacci_number_gen_id = osThreadCreate(osThread(thread_fibonacci_number_gen), NULL);
	
	// Start the kernel
	osTimerStart(preempt_id, 1);
	osKernelStart();	
	osDelay(osWaitForever);
}