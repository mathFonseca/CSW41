/* Include standards */
#include "cmsis_os.h"
#include "TM4C129.h" // Device header
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

/* Include drivers */
#include "grlib/grlib.h"
#include "driverlib/timer.h"
#include "inc/hw_ints.h"
#include "cfaf128x128x16.h"
#include "display.h"
#include "buzzer.h"
#include "uart.h"
#include "pwm.h"
#include "led.h"

/* Defines */
#define N_HARMONICAS 200
#define TAXA 128
#define GANTT 0


/* Timer */
void timer_callback(void const *arg){}
osTimerDef(Timer,timer_callback);
osTimerId Timer_ID;

/* Definicao dos Mails*/
// Mail utilizado pelo UART
// Mensagem UART
osMailQId mid_UART_ID;
osMailQDef (MailQueue, 5, char);

osMailQId mail_UART_tratainfo_ID;
osMailQDef (mail_UART_tratainfo, 5, info_menu*);

//Definição das filas
// Msg plot
osMessageQId(display_queue_ID);
osMessageQDef(display_queue, 5, uint8_t);

osMessageQId(freq_ID);
osMessageQDef(frequencia, 5, uint8_t);

osMessageQId(amplitude_ID);
osMessageQDef(amplitude, 5, uint8_t);

osMessageQId(opt_ID);
osMessageQDef(wavetype, 5, uint8_t);
// Msg PWM
osMessageQId(PWM_queue_ID);
osMessageQDef(PWM_queue, 5, uint16_t);

// Mensagem_informacao
osMessageQId(msg_UART_tratainfo_ID);
osMessageQDef(msg_UART_tratainfo, 5, char*);

/* Criação das threads*/
void UART							(void const *argument);
void generateSignal (void const *argument);
void PWM							(void const *argument);
void Display  (void const *argument);

// Variável que determina ID das threads
osThreadId UART_ID; 
osThreadId generateSignal_ID; 
osThreadId PWM_ID; 
osThreadId Display_ID; 

// UART com prioridade maior que as demais
osThreadDef (UART, osPriorityAboveNormal, 1, 0);     // thread object
osThreadDef (generateSignal, osPriorityNormal, 1, 0);     // thread object
osThreadDef (PWM, osPriorityNormal, 1, 0);     // thread object
osThreadDef (Display, osPriorityNormal, 1, 0);     // thread object

/*------------------------------------------------------------*/
// Timer Interrupt
/*------------------------------------------------------------*/
void TIMER0A_Handler(void){	
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}
 
/*------------------------------------------------------------*/
// Timer Inicializacao
/*------------------------------------------------------------*/
void Timer_Init(){
    // Ativa os perifericos
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Habilita interrupcao dos processos
    IntMasterEnable();

    // Configura o tempo do timer
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()*5);

    // Habilita interrupcoes
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Ativa o Timer
    TimerEnable(TIMER0_BASE, TIMER_A);
}
/*------------------------------------------------------------*/
// Configuracao do CYCCNT
/*------------------------------------------------------------*/
void DWT_Config(){
	/* DWT (Data Watchpoint and Trace) registers, only exists on ARM Cortex with a DWT unit */
	
	/*!< DWT Control register */
	#define KIN1_DWT_CONTROL             (*((volatile uint32_t*)0xE0001000))
	
	/*!< CYCCNTENA bit in DWT_CONTROL register */
	#define KIN1_DWT_CYCCNTENA_BIT       (1UL<<0)
	
	/*!< DWT Cycle Counter register */
	#define KIN1_DWT_CYCCNT              (*((volatile uint32_t*)0xE0001004))
	
	/*!< DEMCR: Debug Exception and Monitor Control Register */
	#define KIN1_DEMCR                   (*((volatile uint32_t*)0xE000EDFC))
	
	/*!< Trace enable bit in DEMCR register */
	#define KIN1_TRCENA_BIT              (1UL<<24)
	
	/*!< TRCENA: Enable trace and debug block DEMCR 
		(Debug Exception and Monitor Control Register */
	#define KIN1_InitCycleCounter() \
		KIN1_DEMCR |= KIN1_TRCENA_BIT
	
	/*!< Reset cycle counter */
	#define KIN1_ResetCycleCounter() \
		KIN1_DWT_CYCCNT = 0
	
	/*!< Enable cycle counter */
	#define KIN1_EnableCycleCounter() \
		KIN1_DWT_CONTROL |= KIN1_DWT_CYCCNTENA_BIT
	
	/*!< Disable cycle counter */
	#define KIN1_DisableCycleCounter() \
		KIN1_DWT_CONTROL &= ~KIN1_DWT_CYCCNTENA_BIT
	
	/*!< Read cycle counter register */
	#define KIN1_GetCycleCounter() \
		KIN1_DWT_CYCCNT
}


/*------------------------------------------------------------*/
// Numeros para string
/*------------------------------------------------------------*/
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


/*------------------------------------------------------------*/
// UART Interrupt
/*------------------------------------------------------------*/
void UART0_Handler(void){
	// Variaveis para leitura de dados da UART
	char m;
	UART_read *mailI;

	// Limpa as flags para reiniciar o processo
	osSignalClear(PWM_ID,0x01);
	osSignalClear(UART_ID,0x01);
	osSignalClear(generateSignal_ID,0x03);

	
	#if GANTT == 1
	osMutexWait(mutex_UART_ID,osWaitForever);
	UARTprintstring("UART Handler: active,");
	cycles = KIN1_GetCycleCounter();
	intToString(cycles,cycles_char,30,10,0);
	UARTprintstring(cycles_char);
	UARTprintstring(",");
	osMutexRelease(mutex_UART_ID);
	#endif

	
	// Enquanto o flag de recebimento nao for 
	while((UART0->FR & (1<<4)) != 0);
	UART0->FR &= (0<<4);
	
	// Coloca na variavel e 
	m = UART0->DR;
	
	UART0	->	RIS |= (1<<4);
	
	mailI = (UART_read*)osMailAlloc(mid_UART_ID,0);
	
	if(mailI){
		mailI	-> msg_UART = m;
		osMailPut(mid_UART_ID,mailI);
	}
	
	// Seta o sinal para a entrada da UART
	osSignalSet(UART_ID,0x01);
	
	#if GANTT == 1
	osMutexWait(mutex_UART_ID,osWaitForever);
	cycles = KIN1_GetCycleCounter();
	intToString(cycles,cycles_char,30,10,0);
	UARTprintstring(cycles_char);
	UARTprintstring("\r\n");
	osMutexRelease(mutex_UART_ID);
	#endif
}
/*------------------------------------------------------------*/
// Init perifericos
/*------------------------------------------------------------*/
void init_all(){
	cfaf128x128x16Init();
	// Inicialização da UART
	inicia_UART();
	Timer_Init();
	initScreen();
	PWM_function_init();
	led_init();
}
/*------------------------------------------------------------*/
// Init threads
/*------------------------------------------------------------*/
int Init_Thread (void) {
	
	Display_ID = osThreadCreate (osThread(Display), NULL);
	if (!Display_ID) return(-1);
	
	UART_ID = osThreadCreate (osThread(UART), NULL);
	if (!UART_ID) return(-1);
	
	PWM_ID= osThreadCreate (osThread(PWM), NULL);
	if (!PWM_ID) return(-1);
	
	generateSignal_ID = osThreadCreate (osThread(generateSignal), NULL);
	if (!generateSignal_ID ) return(-1);

	return(0);
}
/**************************************************************/
/*************************************************************/
/*************************************************************/
// THREAD UART
/*------------------------------------------------------------*/
void UART(void const *argument){
	
	// Definicao do meio de mensagem
	char *pMail;
	char mensagem[10];
	float fator = 10.714285714285;
	info_menu *specs_sinal;
	
	// Evento
	osEvent evt;
	
	// Criacao do Mail
	mid_UART_ID = osMailCreate(osMailQ(MailQueue), NULL);
	
	while(true){
		// Espero pelo sinal vindo da interrupcao
		osSignalWait(0x01,osWaitForever);
		osSignalClear(UART_ID,0x01);
		
		#if GANTT == 1
		osMutexWait(mutex_UART_ID,osWaitForever);
		UARTprintstring("UART: active,");
		cycles = osKernelSysTick();
		intToString(cycles,cycles_char,30,10,0);
		UARTprintstring(cycles_char);
		UARTprintstring(",");
		osMutexRelease(mutex_UART_ID);
		#endif
	
		specs_sinal = (info_menu*)osMailCAlloc(mail_UART_tratainfo_ID,osWaitForever);
		specs_sinal->amplitude = 0;
		specs_sinal->freq = 0;
		specs_sinal->opt = 0;
		specs_sinal->flag_freq = false;
		specs_sinal->flag_amp = false;
		specs_sinal->flag_opt = false;
		
		// Coleta o Mail
		evt = osMailGet(mid_UART_ID,0);
		
		if (evt.status == osEventMail) {
			pMail = evt.value.p;
					
		#if GANTT == 0
			UART_printMenu();
	
			if(pMail[0] == '1'){
				specs_sinal->opt = '1'; // Sinaliza o opt a ser trocado
				specs_sinal->flag_opt = true; // Valida a troca de opt
				UARTprintstring("\033[2J"); // Limpa a tela
				UART_printMenu(); // Printa o menu novamente
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal); // Coloca na Mail as infos
			}
			else if(pMail[0] == '2'){
				specs_sinal->opt = '2';
				specs_sinal->flag_opt = true;
				UARTprintstring("\033[2J");
				UART_printMenu();
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == '3'){
				specs_sinal->opt = '3';
				specs_sinal->flag_opt = true;
				UARTprintstring("\033[2J");
				UART_printMenu();
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == '4'){
				specs_sinal->opt = '4';
				specs_sinal->flag_opt = true;
				UARTprintstring("\033[2J");
				UART_printMenu();
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == '5'){
				specs_sinal->opt = '5';
				specs_sinal->flag_opt = true;
				UARTprintstring("\033[2J");
				UART_printMenu();
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}		
			//q - 0Hz | w - 20Hz | e - 50Hz | r - 100Hz | t - 150Hz | y - 200Hz
			else if(pMail[0] == 'q'){
				specs_sinal->freq = 0; // Ajusta o incremento de 5
				specs_sinal->flag_freq = true; // Ativa a flag de troca
				UARTprintstring("\033[2J"); // Limpa a tela
				UART_printMenu(); // Printa o menu
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == 'w'){
				specs_sinal->freq = 20;
				specs_sinal->flag_freq = true;
				UARTprintstring("\033[2J");
				UART_printMenu();
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}	
			else if(pMail[0] == 'e'){
				specs_sinal->freq = 50; // Ajusta o incremento de 5
				specs_sinal->flag_freq = true; // Ativa a flag de troca
				UARTprintstring("\033[2J"); // Limpa a tela
				UART_printMenu(); // Printa o menu
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == 'r'){
				specs_sinal->freq = 100;
				specs_sinal->flag_freq = true;
				UARTprintstring("\033[2J");
				UART_printMenu();
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}	
			else if(pMail[0] == 't'){
				specs_sinal->freq = 150; // Ajusta o incremento de 5
				specs_sinal->flag_freq = true; // Ativa a flag de troca
				UARTprintstring("\033[2J"); // Limpa a tela
				UART_printMenu(); // Printa o menu
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == 'y'){
				specs_sinal->freq = 200;
				specs_sinal->flag_freq = true;
				UARTprintstring("\033[2J");
				UART_printMenu();
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}		
			//a - 0.0V | s - 0.5V | d - 1.0V | f - 1.5V | g - 2.0V | h - 2.5V | j - 3.0V | k - 3.3
			else if(pMail[0] == 'a'){
					specs_sinal->amplitude = 0.0;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == 's'){
					specs_sinal->amplitude = 0.5*fator;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == 'd'){
					specs_sinal->amplitude = 1*fator;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}			
			else if(pMail[0] == 'f'){
					specs_sinal->amplitude = 1.5*fator;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);			}
			else if(pMail[0] == 'g'){
					specs_sinal->amplitude = 2*fator;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}	
			else if(pMail[0] == 'h'){
					specs_sinal->amplitude = 2.5*fator;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}	
			else if(pMail[0] == 'j'){
					specs_sinal->amplitude = 3*fator;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}	
			else if(pMail[0] == 'k'){
					specs_sinal->amplitude = 3.3*fator;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}	
			
			
			/*
			else if(pMail[0] == 'W'){
				specs_sinal->freq = 5; // Ajusta o incremento de 5
				specs_sinal->flag_freq = true; // Ativa a flag de troca
				UARTprintstring("\033[2J"); // Limpa a tela
				UART_printMenu(); // Printa o menu
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == 'w'){
				specs_sinal->freq = 1;
				specs_sinal->flag_freq = true;
				UARTprintstring("\033[2J");
				UART_printMenu();
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}			
			else if(pMail[0] == 'S'){
				specs_sinal->freq = -5; // Ajusta o decremento de 1
				specs_sinal->flag_freq = true; // Ativa a flag de troca
				UARTprintstring("\033[2J"); // Limpa a tela
				UART_printMenu(); // Printa o menu
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == 's'){
				specs_sinal->freq = -1;
				specs_sinal->flag_freq = true;
				UARTprintstring("\033[2J");
				UART_printMenu();
				osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == 'D'){
					specs_sinal->amplitude = 5;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}
			else if(pMail[0] == 'd'){
					specs_sinal->amplitude = 1;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}			
			else if(pMail[0] == 'A'){
					specs_sinal->amplitude = -5;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);			}
			else if(pMail[0] == 'a'){
					specs_sinal->amplitude = -1;
					specs_sinal->flag_amp = true;
			    UARTprintstring("\033[2J");
					UART_printMenu();
					osMailPut(mail_UART_tratainfo_ID,(info_menu*)specs_sinal);
			}	*/	

			#endif
		}
		osMailFree(mid_UART_ID,pMail);
		osSignalSet(generateSignal_ID,0x03);
		
	#if GANTT == 1
	osMutexWait(mutex_UART_ID,osWaitForever);
	cycles = osKernelSysTick();
	intToString(cycles,cycles_char,30,10,0);
	UARTprintstring(cycles_char);
	UARTprintstring("\r\n");
	osMutexRelease(mutex_UART_ID);
	#endif
	}
}
/*------------------------------------------------------------*/
// THREAD generateSignal
/*------------------------------------------------------------*/
void generateSignal(void const *argument){
	// Variaveis de plot
	uint8_t data = 64;
	double pi = 3.1415926;
	float tempo = 0;
	float tempo_inc = 0;
	float somatorio = 0;
	int16_t quadrada = 0;
	uint16_t n=0,opt=0,i=0;
	uint8_t freq = 20;
	uint8_t amp = 28;// 0 até 35 - 0 até 3,3V 
	uint32_t cycles, cycles_fim;
	uint32_t clock = osKernelSysTickFrequency;
	bool flag = true;
	
	// Info do sinal
	info_menu *sinal = NULL;
	
	// Mensagem da UART
	// Opcao do menu
	osEvent evt;
	char opt_menu; 
	
	// Estabelece canal de mensagem
	mail_UART_tratainfo_ID = osMailCreate(osMailQ(mail_UART_tratainfo),NULL);
	
	// Timer Create
	Timer_ID = osTimerCreate(osTimer(Timer),osTimerOnce,NULL);
	
	while(true){	
		
		// Espera os sinais
		osSignalWait(0X03,osWaitForever);
		osSignalClear(generateSignal_ID,0x03);
		

		#if GANTT == 1
		osMutexWait(mutex_UART_ID,osWaitForever);
		UARTprintstring("generateSignal: active,");
		cycles = osKernelSysTick();
		intToString(cycles,cycles_char,30,10,0);
		UARTprintstring(cycles_char);
		UARTprintstring(",");
		osMutexRelease(mutex_UART_ID);
		#endif

		// Espera pouco por novas mensagens na UART
		// Recebe a mensagem da fila
		
		evt = osMailGet(mail_UART_tratainfo_ID,0); // MILISSEGUNDO
	
		if(evt.status == osEventMail){
			tempo = 0;
			flag = true;
			sinal = (info_menu*)evt.value.v;
			if(sinal->flag_opt == true)
				opt = sinal->opt;
			if(sinal->flag_freq == true ){
				freq = (double)sinal->freq;
				if(freq < 1)
					freq = 1;
				if(freq > 200)
					freq = 200;
			}
			if(sinal->flag_amp == true ){
				amp = (double)sinal->amplitude;
				if(amp < 0)
					amp = 0;
				if(amp > 35)
					amp = 35;
			}
			osMessagePut(freq_ID,freq,0);
			osMessagePut(amplitude_ID,amp,0);
			osMessagePut(opt_ID,opt,0);
		}
		


		/* ----------------------
		------- FORMAS DE ONDA---------
		-------------------------*/
		//SENOIDAL
		if(opt == '1'){
			data = amp*sin(tempo*2.0*pi*freq)+85;					
		}
		//QUADRADA
		else if(opt == '2'){
			  quadrada = amp*sin(tempo*2.0*pi*freq);
				
				if(quadrada <= 0)
						data = amp + 85  ;
				else if(quadrada > 0)
						data = 85 - amp ;
		}

		//DENTE DE SERRA
		else if(opt == '3'){
			somatorio = (((-2*amp)/pi)*atan(1/(tan(pi*freq*tempo))));
			data = 85 + somatorio;		
		}

		//TRIANGULAR
		else if(opt == '4'){
			somatorio = (((2*amp)/pi)*asin(sin(2*pi*freq*tempo)));
			data = 85 + somatorio;		
		}

		//TRAPEIZODAL
		else if(opt == '5'){
			somatorio = (((2*amp)/pi)*asin(sin(2*pi*freq*tempo))) + (((2*amp)/pi)*asin(sin(2*pi*freq*tempo+pi/2)));
			data = 85 + somatorio;	
		}

		//Caso o usuario digite outro valor
		else{
			data = -1;
		}
		// Zera variaveis de Fourier
		somatorio = 0;

		//osTimerStart(Timer_ID,1); // 100 = 100us - <100 -> 7.4us
		//osDelay(5);
		//while(osTimerStop(Timer_ID) != osOK);		

		/* Solucao com tempo de delay*/
		// Tempo
		tempo_inc = (float) osKernelSysTick();
		tempo_inc -= (float) cycles;
		tempo_inc = (float) tempo_inc/clock;
				
		tempo += tempo_inc;		
		
		// Recolhe número de ciclos inicial
		cycles = osKernelSysTick();	
		
		// Coloca as mensagens na fila
		osMessagePut(display_queue_ID, data, 0);
		osMessagePut(PWM_queue_ID,(uint16_t)1020*abs(120-data), 0);
		
		// Limpa mail
		osMailFree(mail_UART_tratainfo_ID,(info_menu*)sinal);
		
		// Seta os sinais necessarios
		osSignalSet(PWM_ID,0x01);
		osSignalSet(Display_ID,0x01);

	#if GANTT == 1
	osMutexWait(mutex_UART_ID,osWaitForever);
	cycles = osKernelSysTick();
	intToString(cycles,cycles_char,30,10,0);
	UARTprintstring(cycles_char);
	UARTprintstring("\r\n");
	osMutexRelease(mutex_UART_ID);
	#endif
	}
}
/*------------------------------------------------------------*/
// THREAD PWM
/*------------------------------------------------------------*/ 
void PWM(void const *argument){
	osEvent event;
	uint16_t amp=0;
	PWM_per_set(200);
	PWM_enable(true);
	PWM_queue_ID = osMessageCreate(osMessageQ(PWM_queue),NULL);	
	while(true){
		// Espera pelo sinal do generateSignal
		osSignalWait(0x01,osWaitForever);
		osSignalClear(PWM_ID,0x01);
		
		#if GANTT == 1
		osMutexWait(mutex_UART_ID,osWaitForever);
		UARTprintstring("PWM: active,");
		cycles = osKernelSysTick();
		intToString(cycles,cycles_char,30,10,0);
		UARTprintstring(cycles_char);
		UARTprintstring(",");
		osMutexRelease(mutex_UART_ID);
		#endif
	
		event = osMessageGet(PWM_queue_ID,osWaitForever);

		amp = (uint16_t) event.value.v;

		PWM_amplitude_set(amp);
		
		
		// Seta o sinal da trata informacao
		osSignalSet(generateSignal_ID,0x01);
		
	#if GANTT == 1
	osMutexWait(mutex_UART_ID,osWaitForever);
	cycles = osKernelSysTick();
	intToString(cycles,cycles_char,30,10,0);
	UARTprintstring(cycles_char);
	UARTprintstring("\r\n");
	osMutexRelease(mutex_UART_ID);
	#endif
		
	}

}

/*------------------------------------------------------------*/
// THREAD Display
/*------------------------------------------------------------*/
void Display(void const *argument){
	
	osEvent event;
	int i=0,j=0;
	uint8_t data;
	uint8_t data_antiga = 0;
	
	uint8_t counter = 0;
	
	float taxa_zoom;
	
	uint8_t freq,amp, wavetype;
	float amplitude_real;
	char freq_char[5],amp_char[5], wv_char[5];
	
	display_queue_ID = osMessageCreate(osMessageQ(display_queue), NULL);
	freq_ID = osMessageCreate(osMessageQ(frequencia), NULL);
	amplitude_ID = osMessageCreate(osMessageQ(amplitude), NULL);
	opt_ID = osMessageCreate(osMessageQ(wavetype), NULL);
	while(true){
		osSignalWait(0x01,osWaitForever);
		osSignalClear(Display_ID,0x01);
		
		#if GANTT == 1
		osMutexWait(mutex_UART_ID,osWaitForever);
		UARTprintstring("Display: active,");
		cycles = osKernelSysTick();
		intToString(cycles,cycles_char,30,10,0);
		UARTprintstring(cycles_char);
		UARTprintstring(",");
		osMutexRelease(mutex_UART_ID);
		#endif
		
		data_antiga = data;
		
		// Frequencia na tela
		event = osMessageGet(freq_ID, 0);
		if(event.status == osEventMessage){
			freq = (uint8_t)event.value.v;		
			intToString(freq,freq_char,-1,10,0);
		}
		// Amplitude na tela
		event = osMessageGet(amplitude_ID, 0);
		if(event.status == osEventMessage){
			amp = (uint8_t)event.value.v;
			// regra de três para ajustar amplitude real
			amplitude_real = ((float)amp)*(0.094);
			floatToString(amplitude_real,amp_char,5,10,0,2);
			drawFreqAmp(freq_char, amp_char);	
		}
		
		// Wavetype na tela
		event = osMessageGet(opt_ID, 0);
		if(event.status == osEventMessage){
			wavetype = (uint8_t)event.value.v;
			drawWaveType(wavetype);	
		}
		
		
		// Melhor para altas frequencias
		//taxa_zoom = 1 - ((float)freq/200);
		// Normal
		taxa_zoom = 1;
		
		// Se passar do limite da tela.
		i++;
		if( i >= 128*taxa_zoom)
			i=0;
		
		// Desenha frequencia e amplitude
		//Coleta dados da mensagem
		event = osMessageGet(display_queue_ID, 0);
		if(event.status == osEventMessage){
			data = (uint8_t)event.value.v;
			drawFunction(data_antiga,data,i/taxa_zoom);
		}
		
//		if(data_antiga > 3*data || data_antiga < 3*data)
//			drawFunction(data_antiga,data,i);
//		else
//			drawFunction(data,data,i);
			
		// Seta o sinal da trata informacao
		osSignalSet(generateSignal_ID,0X02);
		
	#if GANTT == 1
	osMutexWait(mutex_UART_ID,osWaitForever);
	cycles = osKernelSysTick();
	intToString(cycles,cycles_char,30,10,0);
	UARTprintstring(cycles_char);
	UARTprintstring("\r\n");
	osMutexRelease(mutex_UART_ID);
	#endif
	}


}
/*------------------------------------------------------------*/
// MAIN
/*------------------------------------------------------------*/
int main() {
	
	// Inicializa o sistema
	SystemInit();
	
	// Inicializa o Kernel
	osKernelInitialize();	

	//Initializing all peripherals
	init_all();
	
	SystemCoreClockUpdate();
	// Inicializa as Threads
	if(Init_Thread()==-1)
		return 0;
		// Mensagens de erro de inicialização
	
	// Inicializa o Kernel, junto com as threads
	osKernelStart();
	
	//Main aguarda para sempre
	osDelay(osWaitForever);

}
