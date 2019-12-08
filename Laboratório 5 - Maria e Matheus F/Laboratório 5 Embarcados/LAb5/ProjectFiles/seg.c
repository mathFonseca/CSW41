
#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include "grlib/grlib.h"
#include "cfaf128x128x16.h"
#include "buttons.h"
#include <math.h>


#define PI 3.14159265359
#define PI2	9.8696044010
#define TEMPO 20
#define GANNT 1

// To print on the screen
tContext sContext;

//Mutex
osMutexId mutex_display;
osMutexDef(mutex_display);

// Thread IDs
osThreadId  thread_scheduler_id,
						thread_task_A_id,
						thread_task_B_id,
						thread_task_C_id,
						thread_task_D_id,
						thread_task_E_id,
						thread_task_F_id;

// Timer
void preempt_callback() {
	osSignalSet(thread_scheduler_id, 0x01);
}
osTimerDef(preempt, preempt_callback);
osTimerId preempt_id;//_a, preempt_id_b ,preempt_id_c ,preempt_id_d ,preempt_id_e ,preempt_id_f;

typedef enum {
	NOT_READY,
	READY,
	EXECUTING
} task_status;

typedef struct {
	osThreadId thread_id;			// ID da Thread
	int static_priority;			// Prioridade inicial
	int current_priority;			// Prioridade dinamica
	uint64_t expected_duration;	// Dura??o esperada (proveniente de testes isolados)
	float deadline;						// Deadline conforme especificado
	uint8_t ready_counter;		// ?
	uint8_t ready_max;				// ?
	task_status status;				// Status da Thread (vide enum)
	uint32_t start_tick;			// ?
	uint16_t task_id;
} task_t;

// Tasks
task_t task_a, task_b, task_c, task_d, task_e, task_f;
task_t *tasks[6], *execution_q[6];

/*----------------------------------------------------------------------------
 *  Mail
 *---------------------------------------------------------------------------*/

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

/* iniiTasks: configura cada tarefa conforme as especifica??es do trabalho e outros
	parametros medidos externamente.
*/
void initTasks() {
	// Initialize the tasks
	task_a.thread_id = thread_task_A_id;
	task_a.static_priority = 10;
	task_a.current_priority = task_a.static_priority;
	task_a.deadline = 0.7;
	task_a.ready_max = 1000/(8*TEMPO);
	task_a.ready_counter = task_a.ready_max;
	task_a.status = NOT_READY;
	task_a.expected_duration = 639907;
	// result: 66306 - correto
	task_a.task_id = 0;

	task_b.thread_id = thread_task_B_id;
	task_b.static_priority = 0;
	task_b.current_priority = task_b.static_priority;
	task_b.deadline = 0.5;
	task_b.ready_max = 1000/(2*TEMPO);
	task_b.ready_counter = task_b.ready_max;
	task_b.status = NOT_READY;
	task_b.expected_duration = 915000028;//
	// result: 5 - correto: 64
	task_b.task_id = 1;

	task_c.thread_id = thread_task_C_id;
	task_c.static_priority = -30;
	task_c.current_priority = task_c.static_priority;
	task_c.deadline = 0.3;
	task_c.ready_max = 1000/(5*TEMPO);
	task_c.ready_counter = task_c.ready_max;
	task_c.status = NOT_READY;
	task_c.expected_duration = 51000467;//
	// result: 73 - correto: 76,86
	task_c.task_id = 2;

	task_d.thread_id = thread_task_D_id;
	task_d.static_priority = 0;
	task_d.current_priority = task_d.static_priority;
	task_d.deadline = 0.5;
	task_d.ready_max = 1000/(1*TEMPO);
	task_d.ready_counter = task_d.ready_max;
	task_d.status = NOT_READY;
	task_d.expected_duration = 630316000;//
	// result: 1 - correto: 1,87
	task_d.task_id = 3;

	task_e.thread_id = thread_task_E_id;
	task_e.static_priority = -30;
	task_e.current_priority = task_e.static_priority;
	task_e.deadline = 0.3;
	task_e.ready_max = 1000/(6*TEMPO);
	task_e.ready_counter = task_e.ready_max;
	task_e.status = NOT_READY;
	task_e.expected_duration = 900100000;//
	// result: 49793 - correto: 49841,5
	task_e.task_id = 4;

	task_f.thread_id = thread_task_F_id;
	task_f.static_priority = -100;
	task_f.current_priority = task_f.static_priority;
	task_f.deadline = 0.1;
	task_f.ready_max = 1000/(10*TEMPO);
	task_f.ready_counter = task_f.ready_max;
	task_f.status = NOT_READY;
	task_f.expected_duration = 911034000;//
	// result: 20 certo: 26
	task_f.task_id = 5;

	tasks[0] = &task_a;
	tasks[1] = &task_b;
	tasks[2] = &task_c;
	tasks[3] = &task_d;
	tasks[4] = &task_e;
	tasks[5] = &task_f;
}

// init_all: inicializa perif?ricos necess?rios.
void init_all(){
	cfaf128x128x16Init();
}

void init_display_context(){
	GrContextInit(&sContext, &g_sCfaf128x128x16);

	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);

	GrContextForegroundSet(&sContext, ClrYellow);
	GrContextBackgroundSet(&sContext, ClrBlack);
}

// fatorial: auto explicativo.
int fatorial(uint32_t x){
	uint8_t i;
	int result;
	result = 1;
	for(i=1; i<=x; i++)
		result *= i;

	return result;
}

void print_painel(){
		GrStringDraw(&sContext, "ID: ", -1, 0, (sContext.psFont->ui8Height+2)*0, true);
		GrStringDraw(&sContext, "PRIO: ", -1, 0, (sContext.psFont->ui8Height+2)*1, true);
		GrStringDraw(&sContext, "TPRX: ", -1, 0, (sContext.psFont->ui8Height+2)*2, true);
		GrStringDraw(&sContext, "STAT: ", -1, 0, (sContext.psFont->ui8Height+2)*3, true);
		GrStringDraw(&sContext, "PCT%: ", -1, 0, (sContext.psFont->ui8Height+2)*4, true);
		GrStringDraw(&sContext, "ATRA: ", -1, 0, (sContext.psFont->ui8Height+2)*5, true);
		GrStringDraw(&sContext, "FILA: ", -1, 0, (sContext.psFont->ui8Height+2)*6, true);
		GrStringDraw(&sContext, "FALT: ", -1, 0, (sContext.psFont->ui8Height+2)*7, true);
}

void print_fila(uint8_t ready_tasks){
	uint8_t i;
	char pbuf[10];
	GrStringDraw(&sContext, "                ", -1, 30+(i*8), (sContext.psFont->ui8Height+2)*6, true);
	for(i = 0; i < ready_tasks; i++){
		intToString(execution_q[i]->task_id, pbuf,  10, 10, 4);
		GrStringDraw(&sContext, (char*)pbuf, -1, 30+(i*8), (sContext.psFont->ui8Height+2)*6, true);
	}
}
// Escalonador do sistema.
void thread_scheduler(void const *argument) {
	uint8_t i, entries = 0, ready_tasks = 0;
	uint32_t execution_ticks;
	uint32_t atraso;
	char pbuf[100];
	osEvent evt;
	task_ready_msg *msg;
	int faultCounter[6];
	
	initTasks();

	faultCounter[0] = 0;
	faultCounter[1] = 0;
	faultCounter[2] = 0;
	faultCounter[3] = 0;
	faultCounter[4] = 0;
	faultCounter[5] = 0;
	
	#if GANNT == 1
		print_painel();
	#endif

	atraso = 0;
	while(1) {
		// Verifica se alguma task terminou de executar
		evt = osMailGet(task_ready_q_id, 0);
		if(evt.status == osEventMail) {
			msg = (task_ready_msg *) evt.value.p;
			if(msg->task->status == EXECUTING && msg->status == NOT_READY) {
				execution_ticks = osKernelSysTick() - msg->task->start_tick;

				#if GANNT == 1
					osMutexWait(mutex_display,0);
					//ID
					intToString(msg->task->task_id, pbuf,  10, 10, 1);
					GrStringDraw(&sContext, (char*)pbuf, -1, 30, (sContext.psFont->ui8Height+2)*0, true);
					//PRIORIDADE
					intToString(msg->task->current_priority, pbuf,  10, 10, 4);
					GrStringDraw(&sContext, (char*)pbuf, -1, 30, (sContext.psFont->ui8Height+2)*1, true);
					//TEMPO RELAXAMENTO
					intToString(msg->task->expected_duration - execution_ticks, pbuf,  10, 10, 4);
					GrStringDraw(&sContext, (char*)pbuf, -1, 30, (sContext.psFont->ui8Height+2)*2, true);
					//ESTADO
					intToString(msg->task->status, pbuf,  10, 10, 1);
					GrStringDraw(&sContext, (char*)pbuf, -1, 30, (sContext.psFont->ui8Height+2)*3, true);
					//PERCENTUAL - limpar
					GrStringDraw(&sContext, "100", -1, 30, (sContext.psFont->ui8Height+2)*4, true);
					//ATRASO
					atraso =(execution_ticks - (msg->task->expected_duration * (1 + msg->task->deadline))); 
					if( atraso >= 0 )
					{
						intToString(atraso, pbuf, 10, 10, 1);
						GrStringDraw(&sContext, "              ", -1, 30, (sContext.psFont->ui8Height+2)*5, true);	
						GrStringDraw(&sContext, (char*)pbuf, -1, 30, (sContext.psFont->ui8Height+2)*5, true);
					}
					else
					{
						GrStringDraw(&sContext, "              ", -1, 30, (sContext.psFont->ui8Height+2)*5, true);						
					}
					//FILA
					print_fila(ready_tasks);
					//FALTAS
					intToString(faultCounter[msg->task->task_id], pbuf,  10, 10, 4);
					GrStringDraw(&sContext, (char*)pbuf, -1, 30, (sContext.psFont->ui8Height+2)*7, true);
					//ticks
					/*
					intToString(execution_ticks, pbuf,  10, 10, 4);
					GrStringDraw(&sContext, "              ", -1, 30, (sContext.psFont->ui8Height+2)*8, true);
					GrStringDraw(&sContext, (char*)pbuf, -1, 30, (sContext.psFont->ui8Height+2)*8, true);
					*/
					osMutexRelease(mutex_display);
				#endif

				// No caso de Master Fault, mata o sistema.
				if(execution_ticks > (msg->task->expected_duration * (1 + msg->task->deadline))
					&& msg->task->static_priority == -100) {
						#if GANNT == 1
							osMutexWait(mutex_display,0);
							GrStringDraw(&sContext, "Master fault", -1, 0, (sContext.psFont->ui8Height+2)*12, true);
							osMutexRelease(mutex_display);						
						#endif
						faultCounter[msg->task->task_id] +=1;
						while(0) {}
				}
				// No caso de Secondary Fault por lentidao, aumenta prioridade
				else if(execution_ticks > (msg->task->expected_duration * (1 + msg->task->deadline)) && msg->task->static_priority > -100) {
						// Atualiza a tarefa
						if(msg->task->current_priority != -100)
							msg->task->current_priority -= 10;
						
						#if GANNT == 1
							osMutexWait(mutex_display,0);						
							GrStringDraw(&sContext, "Secondary Fault P+", -1, 0, (sContext.psFont->ui8Height+2)*12, true);
							osMutexRelease(mutex_display);						
						#endif

						faultCounter[msg->task->task_id] +=1;
				}
				// No caso de Secondary Fault por rapidez, diminuiu prioridade
				else if(execution_ticks < (msg->task->expected_duration * (1 + msg->task->deadline)) / 2 && msg->task->static_priority > -100) {
						if(msg->task->current_priority != 100)
							msg->task->current_priority += 10;
						#if GANNT == 1
							osMutexWait(mutex_display,0);						
							GrStringDraw(&sContext, "Secondary Fault P-", -1, 0, (sContext.psFont->ui8Height+2)*12, true);
							osMutexRelease(mutex_display);						
						#endif

						faultCounter[msg->task->task_id] +=1;
				}
				// Se não acontecer nenhum dos casos, imprime uma "limpeza" na tela
				else{
					#if GANNT == 1
						osMutexWait(mutex_display,0);					
						GrStringDraw(&sContext, "#####################", -1, 0, (sContext.psFont->ui8Height+2)*12, true);
						osMutexRelease(mutex_display);
					#endif
				}

			} // fim if 

			msg->task->status = msg->status;
			msg->task->ready_counter = msg->task->ready_max;
			//msg->task->current_priority = msg->task->static_priority;

			for(i = 0; i < ready_tasks; i++)
				execution_q[i] = execution_q[i + 1];
			ready_tasks--;
			osMailFree(task_ready_q_id, msg);
		} // fim if

		// A cada TEMPO ms...
		if(entries >= TEMPO) 
		{
			entries = 0;
			ready_tasks = 0;
			for(i = 0; i < 6; i++) 
			{
				tasks[i]->ready_counter--;
				if(tasks[i]->ready_counter <= 0) 
				{
					tasks[i]->status = READY;
					execution_q[ready_tasks] = tasks[i];
					ready_tasks++;
				}
			}
			sortTasks(execution_q, ready_tasks);
		}

		if(execution_q[0]->status != EXECUTING && execution_q[0]->status == READY) {
			execution_q[0]->status = EXECUTING;
			tasks[i]->start_tick = osKernelSysTick();
			osSignalSet(execution_q[0]->thread_id, 0x01);
		}

		entries++;
		osSignalWait(0x01, osWaitForever);
	} // fim do while
}
osThreadDef(thread_scheduler, osPriorityNormal, 1, 0);

// Implementa??es das Threads
void thread_task_A(void const *argument) {

	uint8_t i;
	uint64_t result;
	float porcent;
	char pbuf[10];
	task_ready_msg *msg;

	
	while(1) {
		osSignalWait(0x01, osWaitForever);
		result = 0;
		
		for(i=0; i<=256; i++)
		{
			result +=i+(i+2);
			/*
			if(i%32 == 0)
			{ //8 atualizacoes
				porcent=100*i/256;
				intToString(porcent, pbuf,  10, 10, 4);
				#if GANNT == 1
					osMutexWait(mutex_display,0);
					GrStringDraw(&sContext, "entrou na 0", -1, 30, (sContext.psFont->ui8Height+2)*9, true);
					GrStringDraw(&sContext, pbuf, -1,30, (sContext.psFont->ui8Height+2)*4, true);
					osMutexRelease(mutex_display);
				#endif
			}
			*/
		}
		//faz algum tipo de verifica??o

		msg = (task_ready_msg *) osMailAlloc(task_ready_q_id, osWaitForever);
		msg->task = tasks[0];
		msg->status = NOT_READY;
		osMailPut(task_ready_q_id, msg);
		
	}
}
osThreadDef(thread_task_A, osPriorityNormal, 1, 0);

void thread_task_B(void const *argument) {

		uint8_t i;
		float result;
		float porcent;
		char pbuf[10];
		task_ready_msg *msg;

		while(1) {
			osSignalWait(0x01, osWaitForever);
			result = 0;
			for(i=1; i<=16; i++)
			{
				// 2^i em C ? um XOR e n?o ? exponencial. Ai fiz com pow do math.h
				result += pow(2,i)/(fatorial(i));
				/*
				if(i%2 == 0){	//8 atualizacoes
					porcent=100*i/16;
					intToString(porcent, pbuf,  10, 10, 4);
					#if GANNT == 1
						osMutexWait(mutex_display,0);
						GrStringDraw(&sContext, pbuf, -1,30, (sContext.psFont->ui8Height+2)*4, true);
						osMutexRelease(mutex_display);
					#endif
				}
				*/
			}
			//faz algum tipo de verifica??o
			msg = (task_ready_msg *) osMailAlloc(task_ready_q_id, osWaitForever);
			msg->task = tasks[1];
			msg->status = NOT_READY;
			osMailPut(task_ready_q_id, msg);
		}
}
osThreadDef(thread_task_B, osPriorityNormal, 1, 0);

void thread_task_C(void const *argument) {

	uint8_t i;
	float result;
	float porcent;
	char pbuf[10];
	task_ready_msg *msg;

	while(1) {
		osSignalWait(0x01, osWaitForever);
		result = 0;

		for(i=0; i<=72; i++){
			result += (i+1)/(i);
			/*
			if(i%9 == 0){	//8 atualizacoes
				porcent=100*i/72;
				intToString(porcent, pbuf,  10, 10, 4);
				#if GANNT == 1
					osMutexWait(mutex_display,0);				
					GrStringDraw(&sContext, pbuf, -1,30, (sContext.psFont->ui8Height+2)*4, true);
					osMutexRelease(mutex_display);
				#endif
			}
			*/
		}

		//faz algum tipo de verifica??o
		msg = (task_ready_msg *) osMailAlloc(task_ready_q_id, osWaitForever);
		msg->task = tasks[2];
		msg->status = NOT_READY;
		osMailPut(task_ready_q_id, msg);
	}
}
osThreadDef(thread_task_C, osPriorityNormal, 1, 0);

void thread_task_D(void const *argument) {

	uint8_t i;
	float result;
	float porcent;
	char pbuf[10];
	task_ready_msg *msg;

	while(1) {
		osSignalWait(0x01, osWaitForever);
		result = 0;
		/*
		#if GANNT == 1
			osMutexWait(mutex_display,0);
			GrStringDraw(&sContext, "0", -1,30, (sContext.psFont->ui8Height+2)*4, true);
			osMutexRelease(mutex_display);
		#endif
		*/
		result = 1 + (5/fatorial(3) + (5/(fatorial(5))) + (5/(fatorial(7))) + (5/(fatorial(9))));
		/*
		#if GANNT == 1
			osMutexWait(mutex_display,0);
			GrStringDraw(&sContext, "100", -1,30, (sContext.psFont->ui8Height+2)*4, true);
			osMutexRelease(mutex_display);
		#endif
		*/

		//faz algum tipo de verifica??o
		msg = (task_ready_msg *) osMailAlloc(task_ready_q_id, osWaitForever);
		msg->task = tasks[3];
		msg->status = NOT_READY;
		osMailPut(task_ready_q_id, msg);
	}
}
osThreadDef(thread_task_D, osPriorityNormal, 1, 0);

void thread_task_E(void const *argument) {

	uint8_t i;
	float result;
	float porcent;
	char pbuf[10];
	task_ready_msg *msg;

	while(1) {
		osSignalWait(0x01, osWaitForever);
		result = 0;
		for(i=1; i<=100; i++){
			result += i * PI * PI;
			/*
			if(i%10 == 0){	//10 atualizacoes
				porcent=100*i/100;
				intToString(porcent, pbuf,  10, 10, 4);
				#if GANNT == 1
					osMutexWait(mutex_display,0);
					GrStringDraw(&sContext, pbuf, -1,30, (sContext.psFont->ui8Height+2)*4, true);
					osMutexRelease(mutex_display);
				#endif
			}
			*/
		}
		//faz algum tipo de verifica??o
		msg = (task_ready_msg *) osMailAlloc(task_ready_q_id, osWaitForever);
		msg->task = tasks[4];
		msg->status = NOT_READY;
		osMailPut(task_ready_q_id, msg);
	}
}
osThreadDef(thread_task_E, osPriorityNormal, 1, 0);

void thread_task_F(void const *argument) {

	uint8_t i;
	uint64_t result;
	float porcent;
	char pbuf[10];
	task_ready_msg *msg;

	while(1) {
		osSignalWait(0x01, osWaitForever);
		result = 0;

		for(i=1; i<=128; i++)
		{
				result += (i*i*i)/(1<<i);
			/*
				if(i%16 == 0){	//8 atualizacoes
					porcent=100*i/128;
					intToString(porcent, pbuf,  10, 10, 4);
					#if GANNT == 1
						osMutexWait(mutex_display,0);	
						GrStringDraw(&sContext, pbuf, -1, 30, (sContext.psFont->ui8Height+2)*4, true);
						osMutexRelease(mutex_display);
					#endif
				}
			*/
				// Considera??es sobre essa mudan?a:
				// 2^128 ? muito grande. Na verdade, no meio do caminho o 1/2^128 ? praticamente 0.
				// O ruim ? que o pow demora muito, ent?o pra deixar a tarefa mais r?pido, parei o for no meio do caminho mesmo
		}
		//faz algum tipo de verifica??o
		msg = (task_ready_msg *) osMailAlloc(task_ready_q_id, osWaitForever);
		msg->task = tasks[5];
		msg->status = NOT_READY;
		osMailPut(task_ready_q_id, msg);
	}
}
osThreadDef(thread_task_F, osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/
int main (void) {

	//Initializing all peripherals
	
	#if GANNT == 1
		init_all();
		init_display_context();
	#endif


	// Initialize the kernel
	osKernelInitialize();

	//Create the mutex
	mutex_display = osMutexCreate(osMutex(mutex_display));
	
	// Create the mail queues
	task_ready_q_id = osMailCreate(osMailQ(task_ready_q), NULL);

	// Create the timers
	preempt_id = osTimerCreate(osTimer(preempt), osTimerPeriodic, (void*) 0);

	// Create the threads
	thread_scheduler_id	= osThreadCreate(osThread(thread_scheduler), NULL);
	thread_task_A_id	= osThreadCreate(osThread(thread_task_A), NULL);
	thread_task_B_id = osThreadCreate(osThread(thread_task_B), NULL);
	thread_task_C_id = osThreadCreate(osThread(thread_task_C), NULL);
	thread_task_D_id = osThreadCreate(osThread(thread_task_D), NULL);
	thread_task_E_id = osThreadCreate(osThread(thread_task_E), NULL);
	thread_task_F_id = osThreadCreate(osThread(thread_task_F), NULL);

	// Start the kernel
	osTimerStart(preempt_id, 1);
	osKernelStart();

	osDelay(osWaitForever);
}
