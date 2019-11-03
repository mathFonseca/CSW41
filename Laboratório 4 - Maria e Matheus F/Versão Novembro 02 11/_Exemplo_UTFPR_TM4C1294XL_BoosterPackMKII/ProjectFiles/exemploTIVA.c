
/* Includes */
#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include "grlib/grlib.h"
#include "driverlib/timer.h"
#include "cfaf128x128x16.h"
#include "buttons.h"
#include "buzzer.h"
#include "math.h"
#include "PWM.h"
#include "UART.h"

/* Defines */
#define ZERO 78
#define MAX_V 3.3
#define MAX_F 200
#define DISPLAY_TAM 128
#define MAX_SIGNAL_HEIGHT 49
#define PWM_MAX_TENSION 0x7FFF
#define PWM_PER 0XFFFF
#define L1 32
#define L2 64
#define L3 96
#define SCALE_DEFAULT 4
#define F_PWM 2560
#define PI 3.1415926

#define GANTT 0


tContext sContext;
/* Timer */
void timer_callback(void const *arg){}
osTimerDef(Timer,timer_callback);
osTimerId Timer_ID;
	
// Declaração das Funções das Threads
void t_GenerateSignal (void const *argument);
void t_Display 		    (void const *argument);
void t_PWM 			      (void const *argument);
void t_UART						(void const *argument);

// Struct usada pela Thread UART para armazenar e enviar dados
typedef struct{
	float amplitude;
	uint16_t  waveType;
	uint16_t fn;
	uint16_t h_scale;
}Signal;
osPoolId sig;
osPoolDef(sig, 16, Signal);

typedef struct{
	uint16_t x;
	uint16_t y;
}Ponto;
osPoolId pt;
osPoolDef(pt, 16, Ponto);

typedef struct{
	uint8_t m;
}Msg;
osPoolId msg;
osPoolDef(msg, 16, Msg);

typedef struct{
	float m;
}PWM_Msg;

osPoolId pwm_msg;
osPoolDef(pwm_msg, 16, PWM_Msg);

// Definição Threads
osThreadId t_display_id;
osThreadId t_PWM_id;
osThreadId t_generatesignal_id;
osThreadId t_uart_id;

osThreadDef (t_Display, osPriorityNormal, 1, 0);						// thread object
osThreadDef (t_PWM, osPriorityNormal, 1, 0);     						// thread object
osThreadDef (t_GenerateSignal, osPriorityNormal , 1, 0);    // thread object
osThreadDef (t_UART, osPriorityAboveNormal, 1, 0);    			// thread object

//Definição das filas e suas IDs
osMailQId uart_msg_id;										//uart
osMailQDef(uart_msg, 16, char);
osMailQId usr2gen_id;										//usr2gen
osMailQDef(usr2gen, 16, Signal);
osMessageQId gen2dis_id;										//gen2dis
osMessageQDef(gen2dis, 16, Ponto);
osMessageQId gen2pwm_id;										//gen2pwm
osMessageQDef(gen2pwm, 16, PWM_Msg);

//MUTEX
osMutexId mutex_display_id;
osMutexDef(mutex_display);


//Funções

/*----------------------------------------------------------------------------
 *  Transforming int to string
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



	






void draw_pixel(uint16_t x, uint16_t y){
	GrPixelDraw(&sContext, x, y);
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
void TIMER0A_Handler(void){	
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}
void UART0_Handler(void){
	
		// Variaveis para leitura de dados da UART
		char m;
		Msg *mailI;
		// Limpa as flags para reiniciar o processo
		osSignalClear(t_PWM_id,0x03);
		osSignalClear(t_display_id,0x02);
		osSignalClear(t_uart_id,0x01);
		osSignalClear(t_generatesignal_id,0x05);

		
		// Enquanto o flag de recebimento nao for 
		while((UART0->FR & (1<<4)) != 0);
		
		UART0->FR &= (0<<4);
		
		// Coloca na variavel e 
		m = UART0->DR;
		
		UART0	->	RIS |= (1<<4);
		mailI = (Msg*) osMailAlloc(uart_msg_id,0);
		if(mailI){
			mailI->m = m;
			osMailPut(uart_msg_id, mailI);
		}
		
		// Seta o sinal para a entrada da UART
		osSignalSet(t_uart_id,0x01);
}



// Função que instancia as Threads 
int Init_Thread (void) {
  t_display_id = osThreadCreate (osThread(t_Display), NULL);
	if (!t_display_id) return(-1);
  t_PWM_id = osThreadCreate (osThread(t_PWM), NULL);
	if (!t_PWM_id) return(-1);
	t_generatesignal_id = osThreadCreate (osThread(t_GenerateSignal), NULL);
	if (!t_generatesignal_id) return(-1);
  t_uart_id = osThreadCreate (osThread(t_UART), NULL);
	if (!t_uart_id) return(-1);
	return(0);
}
// função de iniialização dos periféricos
void init_all(){
	cfaf128x128x16Init();
	buzzer_init(); 
	button_init();
	PWM_function_init();
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	UART_Enable();
	SysTick_Init();
	Timer_Init();
}




void PrintCaracteristics(Signal *signal){
		char pbuf_f[10], pbuf_a[10];
		char wave[5];
	
		//WaveType: 0->senoide; 1->triangular; 2->dente de serra; 3->quadrada; 4->trapezoidal
		GrStringDraw(&sContext,"WaveType:", -1, (sContext.psFont->ui8MaxWidth)*0,  (sContext.psFont->ui8Height+2)*0, true);
		if(signal->waveType==0){
				GrStringDraw(&sContext,"Sine     ", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
		}
		if(signal->waveType==1){
				GrStringDraw(&sContext,"Triangle", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
		}
		if(signal->waveType==2){
				GrStringDraw(&sContext,"SawTooth ", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
		}
		if(signal->waveType==3){
				GrStringDraw(&sContext,"Square   ", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
		}
		if(signal->waveType==4){
				GrStringDraw(&sContext,"Trapezoid", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
		}
		
		//frequency
		intToString(signal->fn, pbuf_f, 10, 10, 3);
		GrStringDraw(&sContext,"Freq:", -1, (sContext.psFont->ui8MaxWidth)*0,  (sContext.psFont->ui8Height+2)*1, true);
		GrStringDraw(&sContext,pbuf_f, -1, (sContext.psFont->ui8MaxWidth)*5,  (sContext.psFont->ui8Height+2)*1, true);
		//amp
		floatToString(signal->amplitude, pbuf_a, 10, 10, 1, 1);
		GrStringDraw(&sContext,"Ampl:", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*1, true);
		GrStringDraw(&sContext,pbuf_a, -1, (sContext.psFont->ui8MaxWidth)*15,  (sContext.psFont->ui8Height+2)*1, true);
} 




// THREADS

// Theead que Gera valoresde amplitude eenvia para asdemais Threads
void t_GenerateSignal (void const *argument){
	
		//variaveis utiliadas na comunicação entre thread
		osEvent evt;								
		Signal *signal;
		
		float amp;
		float y[512];
		uint16_t  output;
		uint16_t i;
		uint16_t tam;
		double inc_t;
		float scale;
		bool f0;	//flag para qd a frequencia for 0
		int last_t;
		//inicializacao de valores padrao
		signal = (Signal*) osPoolAlloc(sig);
		signal->amplitude = 2;
		signal->fn = 20;
		signal->h_scale = 4;
		
		amp = (signal->amplitude)*(MAX_SIGNAL_HEIGHT/MAX_V);
		tam = (int) F_PWM/signal->fn;
		inc_t = 0.000393625*(signal->fn);
		scale = signal->h_scale/SCALE_DEFAULT;
					
		for(i=0; i<tam ; i++){
				y[i]=sin(i*(2*PI/tam));
		}
		
		GrContextForegroundSet(&sContext, ClrWhite);

		GrStringDraw(&sContext,"Selecione uma Onda:", -1, (sContext.psFont->ui8MaxWidth)*2,  (sContext.psFont->ui8Height+2)*10, true);
		
		
		last_t = 0;
		
		osSignalWait(0x06,osWaitForever);
		while(1){
			
				evt = osMailGet(usr2gen_id, 0);
				
				//atualizacao da onda
				if(evt.status == osEventMail){
						Signal *newSignal =(Signal*)evt.value.p;
					
						amp = (newSignal->amplitude)*(MAX_SIGNAL_HEIGHT/MAX_V);
						if(newSignal->fn==0){
								tam = (int) F_PWM/newSignal->fn;
						}
						else{
								tam = 1;
						}
						inc_t = 0.000393625*(newSignal->fn);
						scale = newSignal->h_scale/SCALE_DEFAULT;
					
						for(i=0; i<tam ; i++){
							//WaveType: 0->senoide; 1->triangular; 2->dente de serra; 3->quadrada; 4->trapezoidal
								if(newSignal->fn==0){
										y[0]=0;
										if(newSignal->waveType==3){
												y[0]=1;
										}
										break;
								}
								//WaveType=senoide
								else if(newSignal->waveType==0){
										y[i]=sin(i*(2*PI/tam));
								}
								//WaveType=triangular
								else if(newSignal->waveType==1){
										if(i<(tam/4)){
												y[i] = i*(4*inc_t);
										}
										else if(i==(tam/4)){
												y[i]=1;
										}
										else if((i>(tam/4)) && (i<(tam/4)*3)){
												y[i] = 1-(i-(tam/4))*(4*inc_t);
										}
										else if(i==((tam/4)*3)){
												y[i]=-1;
										}
										else if(i>((tam/4)*3)){
												y[i] = -1+(i-((tam/4)*3))*(4*inc_t);
										}
										else{
												y[i]=0;
										}
								}
								//WaveType=Dente de Serra
								else if(newSignal->waveType==2){
										y[i]= (double)i*(inc_t);
								}
								//WaveType=Quadrada
								else if(newSignal->waveType==3){
										if(i<tam/2)
												y[i] = 1;
										else
												y[i] = -1;
								}		
								//WaveType=Trapezoidal
								else if(newSignal->waveType==4){
										if(i<(tam/4)){
												y[i] = i*(4*inc_t);
										}
										else if((i>=(tam/4)) && (i<=(tam/4)*3)){
												y[i] = 1;
										}
										else if(i>((tam/4)*3)){
												y[i] = 1-(i-((tam/4)*3))*(4*inc_t);
										}
										else{
												y[i]=0;
										}
								}
						}	
				
						signal->amplitude = newSignal->amplitude;
						signal->fn = newSignal->fn;
						signal->h_scale = newSignal->h_scale;
						signal->waveType = newSignal->waveType;
						osPoolFree(sig, newSignal);
				}
				GrContextForegroundSet(&sContext, ClrWhite);

				//regiao critica
				osMutexWait(mutex_display_id, osWaitForever);
				PrintCaracteristics(signal);
				osMutexRelease(mutex_display_id);
				//fim da região critica
				
				PWM_amplitude_set(PWM_MAX_TENSION*(signal->amplitude/MAX_V));
				
				for(i=0; i<128; i++){
						double valor_y;
						Ponto *pt_  = (Ponto*)osPoolAlloc(pt);
						PWM_Msg *pwmMsg  = (PWM_Msg*)osPoolAlloc(pwm_msg);
					
						if(signal->fn==0){
								valor_y = y[0];
						}
						else{
								valor_y = y[((int)(last_t*scale))%(tam)];
						}
						output = ZERO + amp*valor_y;
						
						last_t++;
						if(last_t==tam+1){
								last_t = 0;
						}
						pt_->x = i;
						pt_->y  = output;
						pwmMsg->m = valor_y;
						osMessagePut(gen2dis_id, (uint32_t) pt_, osWaitForever);
						osMessagePut(gen2pwm_id, (uint32_t) pwmMsg, osWaitForever);
						SysTick_Wait1ms(1000/tam);
						osSignalWait(0x05,osWaitForever);
						
				}
		}
}

//Thread que recebe um valor de amplitude e plota no displaytambém como valores de frequencia e amplitude
void t_Display (void const *argument){
	
	// variaveis usadas na comunicação entre trheads e plot no display
	osEvent event;
	
	uint16_t j;
	uint16_t altura;
	
	while(1){	
		// Espera por uma mensagem
		event = osMessageGet(gen2dis_id, osWaitForever);
		if (event.status == osEventMessage){
			Ponto *pt_ = (Ponto*)event.value.p;
			
			if(pt_->y<ZERO){
					for(j=ZERO-MAX_SIGNAL_HEIGHT; j<ZERO+MAX_SIGNAL_HEIGHT;j++){
							if((j>=ZERO) && (j<altura)){
									GrContextForegroundSet(&sContext, ClrWhite);
									// inicio sessao critica
									osMutexWait(mutex_display_id, osWaitForever);
									draw_pixel(pt_->x, j);
									osMutexRelease(mutex_display_id);
									// fim sessao critica
							}
							else{
									GrContextForegroundSet(&sContext, ClrBlack);
									// inicio sessao critica
									osMutexWait(mutex_display_id, osWaitForever);
									draw_pixel(pt_->x, j);
									osMutexRelease(mutex_display_id);
									// fim sessao critica
							}
					}
			}
			else if(pt_->y>ZERO){
					for(j=ZERO-MAX_SIGNAL_HEIGHT; j<ZERO+MAX_SIGNAL_HEIGHT;j++){
							if((j<ZERO) && (j>=altura)){
									GrContextForegroundSet(&sContext, ClrWhite);
									// inicio sessao critica
									osMutexWait(mutex_display_id, osWaitForever);
									draw_pixel(pt_->x, j);
									osMutexRelease(mutex_display_id);
									// fim sessao critica
							}
							else{
									GrContextForegroundSet(&sContext, ClrBlack);
									// inicio sessao critica
									osMutexWait(mutex_display_id, osWaitForever);
									draw_pixel(pt_->x, j);
									osMutexRelease(mutex_display_id);
									// fim sessao critica
							}
					}							
			}

			osPoolFree(pt, pt_);	
			osSignalSet(t_generatesignal_id,0x02);
		}
	}	
}
// Recebe um valor de amplitude e reproduz no PWM
void t_PWM (void const *argument){
	float amplitude;
	
	PWM_per_set(100);
	PWM_enable(true);
	
	
	while(1){
		osEvent evt = osMessageGet(gen2pwm_id,osWaitForever);
		
		if (evt.status == osEventMessage){
				PWM_Msg *msg = (PWM_Msg*) evt.value.p;
				amplitude = (msg->m)*(PWM_PER/MAX_V);
				PWM_amplitude_set(amplitude);
				osPoolFree(pwm_msg, msg);
				osSignalSet(t_generatesignal_id,0x03);	
		}
	}
}
// recebe um caractere e muda seus estados de onda enviando informaçoes para as demais trheads
void t_UART (void const *argument){
		osEvent evt;
		char *pMail;
		
	
		while(true){
				Signal *newSignal = (Signal*)osMailAlloc(usr2gen_id, osWaitForever);
				newSignal->amplitude = 2;
			  newSignal->fn = 20;
				newSignal->h_scale=4;
				newSignal->waveType = 0;
				//escolher tipo de onda
				UARTprintMenu(0);
				osSignalWait(0x01,osWaitForever);
				
				evt = osMailGet(uart_msg_id,osWaitForever);
				if (evt.status == osEventMail) {
					pMail = evt.value.p;
					
					switch(*pMail){
							case '0':
								newSignal->waveType=0;
							case '1':
								newSignal->waveType=1;
							case '2':
								newSignal->waveType=2;
							case '3':
								newSignal->waveType=3;
							case '4':
								newSignal->waveType=4;
					}
					osPoolFree(msg, pMail);	
				
					//escolher frequencia
					UARTprintMenu(1);
					osSignalWait(0x01,osWaitForever);

					evt = osMailGet(uart_msg_id,osWaitForever);
					if (evt.status == osEventMail) {
						pMail = evt.value.p;
						switch(*pMail){
								case '0':
									newSignal->fn = 0;
								case '1':
									newSignal->fn = 20;
								case '2':
									newSignal->fn = 50;
								case '3':
									newSignal->fn = 100;
								case '4':
									newSignal->fn = 150;
								case '5':
									newSignal->fn = 200;
						}
						osPoolFree(msg, pMail);
					
						//escolher amplitude
						UARTprintMenu(2);
						osSignalWait(0x01,osWaitForever);

						evt = osMailGet(uart_msg_id,osWaitForever);

						if (evt.status == osEventMail) {
								pMail = evt.value.p;
								switch(*pMail){
										case '0':
											newSignal->amplitude=0.5;
										case '1':
											newSignal->amplitude=1.0;
										case '2':
											newSignal->amplitude=1.5;
										case '3':
											newSignal->amplitude=2.0;
										case '4':
											newSignal->amplitude=2.5;
										case '5':
											newSignal->amplitude=3.0;
										case '6':
											newSignal->amplitude=3.3;
								}
								osPoolFree(msg, pMail);
						}
					}	
				}
				
				newSignal->h_scale = 4;
				
				osMailPut(usr2gen_id, (Signal*)newSignal);
				osSignalSet(t_generatesignal_id, 0x06);
				osDelay(100);
		}
}

int main() {
	
	osKernelInitialize(); 				
	init_all();
	
	if(Init_Thread()!=0)  				// Mensagens de erro de inicialização
		return 0;										// Inicializa o Kernel, junto com as threads
	
	//create memory pool
	sig = osPoolCreate(osPool(sig));
	pt = osPoolCreate(osPool(pt));
	msg = osPoolCreate(osPool(msg));
	pwm_msg = osPoolCreate(osPool(pwm_msg));
	
	//create Mails and Messages
	uart_msg_id = osMailCreate(osMailQ(uart_msg), NULL);
	gen2pwm_id = osMessageCreate(osMessageQ(gen2pwm), NULL);
	gen2dis_id = osMessageCreate(osMessageQ(gen2dis), NULL);
	usr2gen_id = osMailCreate(osMailQ(usr2gen), NULL);
	
	//create  mutex
	mutex_display_id = osMutexCreate(osMutex(mutex_display));
	// Timer Create
	Timer_ID = osTimerCreate(osTimer(Timer),osTimerOnce,NULL);
	
	
	
	osKernelStart();		//Main aguarda para sempre
	
	osDelay(osWaitForever);
}
