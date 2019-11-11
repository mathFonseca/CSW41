/*============================================================================
 *                    Exemplos de utiliza��o do Kit
 *              EK-TM4C1294XL + Educational BooterPack MKII 
 *---------------------------------------------------------------------------*
 *                    Prof. Andr� Schneider de Oliveira
 *            Universidade Tecnol�gica Federal do Paran� (UTFPR)
 *===========================================================================
 * Autores das bibliotecas:
 * 		Allan Patrick de Souza - <allansouza@alunos.utfpr.edu.br>
 * 		Guilherme Jacichen     - <jacichen@alunos.utfpr.edu.br>
 * 		Jessica Isoton Sampaio - <jessicasampaio@alunos.utfpr.edu.br>
 * 		Mariana Carri�o        - <mcarriao@alunos.utfpr.edu.br>
 *===========================================================================*/
#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include "grlib/grlib.h"
#include "driverlib/timer.h"
#include "inc/hw_ints.h"

/*----------------------------------------------------------------------------
 * include libraries from drivers
 *----------------------------------------------------------------------------*/

#include "cfaf128x128x16.h"
#include "math.h"
#include "servo.h"
#include "temp.h"
#include "opt.h"
#include "buttons.h"
#include "buzzer.h"
#include "joy.h"
#include "mic.h"
#include "accel.h"
#include "led.h"
#include "NewUART.h"
#include "NewPWM.h"
#include "displayWaves.h"

//To print on the screen
tContext sContext;

// Timer
osTimerId timer1;

//Mutex
osMutexId mutex_display;
osMutexDef(mutex_display);

// Thread IDs
osThreadId thread_uart_id, thread_math_id, thread_display_id, thread_pwm_id;

/**------------------------------------------------------------------------
			MENSAGENS
*-------------------------------------------------------------------------*/
typedef struct {
	char msg;
} uart_isr_msg;

osMailQDef(uart_mail_q,2,uart_isr_msg);
osMailQId (uart_mail_q_id);

typedef struct {
	int waveType;
	float amplitude;
	float freq;
} math_msg;

osMailQDef(math_mail_q,2,math_msg);
osMailQId (math_mail_q_id);

typedef struct {
	float steps[4000];
	float amplitude;
	float freq;
} pwm_msg;

osMailQDef(pwm_mail_q,2,pwm_msg);
osMailQId (pwm_mail_q_id);

typedef struct {
	float steps[4000];
	enum WaveType waveType;
	float amplitude;
	float freq;
} display_msg;

osMailQDef(display_mail_q,2,display_msg);
osMailQId (display_mail_q_id);



/*----------------------------------------------------------------------------
 *    Initializations
 *---------------------------------------------------------------------------*/

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
}


/*----------------------------------------------------------------------------
 *  Threads
 *---------------------------------------------------------------------------*/
	
void thread_uart(void const *argument){
	osEvent evt;
	math_msg * mathMsg;
	int tipoOnda;
	uart_isr_msg *message = 0;
	
	while(1){
		uart_print_string("\033[2J");
		uart_print_string("\033[0;0H");
		uart_print_string("\n\rSelecione a Forma de Onda:\n\r");
		uart_print_string("1-Quadrada\n\r");
		uart_print_string("2-Senoidal\n\r");
		uart_print_string("3-Triangular\n\r");
		uart_print_string("4-Dente de Serra\n\r");
		uart_print_string("5-Trapezoidal\n\r");
		
		evt = osMailGet(uart_mail_q_id,osWaitForever);
		while(evt.status!=osEventMail)
			evt = osMailGet(uart_mail_q_id,osWaitForever);
		
		mathMsg = (math_msg*)osMailAlloc(math_mail_q_id, osWaitForever);
		
		message = (uart_isr_msg*) evt.value.p;
		
		switch(message->msg) {
			case '1':
				tipoOnda = 1;
				break;
			
			case '2':
				tipoOnda = 2;
				break;
			
			case '3':
				tipoOnda = 3;
				break;
			
			case '4':
				tipoOnda = 4;
				break;
			
			case '5':
				tipoOnda = 5;
				break;
		}

		print_char(message->msg);
		uart_print_string("\n\n");
		uart_print_string("Qual a frequencia?\n\r");
		uart_print_string("1-25Hz\n\r");
		uart_print_string("2-50Hz\n\r");
		uart_print_string("3-75Hz\n\r");
		uart_print_string("4-100Hz\n\r");
		uart_print_string("5-150Hz\n\r");
		uart_print_string("6-200Hz\n\r");
		osMailFree(uart_mail_q_id,message);
		
		evt = osMailGet(uart_mail_q_id,osWaitForever);
		while(evt.status!=osEventMail)
			evt = osMailGet(uart_mail_q_id,osWaitForever);
		
		message = (uart_isr_msg*) evt.value.p;
		
		switch(message->msg) {
			case '1':
				mathMsg->freq = 25.0;
				break;
			
			case '2':
				mathMsg->freq = 50.0;
				break;
			
			case '3':
				mathMsg->freq = 75.0;
				break;
			
			case '4':
				mathMsg->freq = 100.0;
				break;
			
			case '5':
				mathMsg->freq = 150.0;
				break;
			
			case '6':
				mathMsg->freq = 200.0;
				break;
		}
		
		print_char(message->msg);
		uart_print_string("\n\n");
		uart_print_string("Qual a amplitude?\n\r");
		uart_print_string("1-0.5V\n\r");
		uart_print_string("2-1V\n\r");
		uart_print_string("3-1.5V\n\r");
		uart_print_string("4-2.0V\n\r");
		uart_print_string("5-2.5V\n\r");
		uart_print_string("6-3V\n\r");
		uart_print_string("7-3.3V\n\r");
		osMailFree(uart_mail_q_id,message);
		
		evt = osMailGet(uart_mail_q_id,osWaitForever);
		while(evt.status!=osEventMail)
			evt = osMailGet(uart_mail_q_id,osWaitForever);
		
		message = (uart_isr_msg*) evt.value.p;		
		switch(message->msg) {
			case '1':
				mathMsg->amplitude = 0.5;
				break;
			
			case '2':
				mathMsg->amplitude = 1;
				break;
			
			case '3':
				mathMsg->amplitude = 1.5;
				break;
			
			case '4':
				mathMsg->amplitude = 2;
				break;
			
			case '5':
				mathMsg->amplitude = 2.5;
				break;
			
			case '6':
				mathMsg->amplitude = 3.0 ;
				break;
			
			case '7':
				mathMsg->amplitude = 3.3;
				break;
		}
		osMailFree(uart_mail_q_id,message);
		
		if(mathMsg){
			mathMsg	-> waveType = tipoOnda;
			osMailPut(math_mail_q_id,mathMsg); 
		}
		
 }
}
osThreadDef(thread_uart, osPriorityNormal, 1, 0);

void thread_math(void const *argument){
	osEvent evt;
	math_msg *message = 0;
	pwm_msg *pwmMsg;
	float maior=0;
	display_msg *displayMsg;	
	int i,n;
	int stepsNumber;
	while(1){
		pwmMsg = (pwm_msg*)osMailAlloc(pwm_mail_q_id, 0);
		displayMsg = (display_msg*)osMailAlloc(display_mail_q_id, 0);
		for(i=0;i < 4000; i++) {
			pwmMsg->steps[i] = 1000; 
			displayMsg->steps[i] = 1000; 
		}
		evt = osMailGet(math_mail_q_id, osWaitForever);
		
		if(evt.status == osEventMail){
			message = (math_msg*) evt.value.p;			
			displayMsg->amplitude = message->amplitude;
			displayMsg->freq = message->freq;
			
			if(message -> waveType == 1)
			{
				stepsNumber = 100000/message->freq;
				for(i=0;i<=stepsNumber;i++)
				{
					if(i<stepsNumber/2)
						pwmMsg->steps[i] = message->amplitude;
					else
						pwmMsg->steps[i] = 0;
					displayMsg->steps[i] = pwmMsg->steps[i];
				}
				displayMsg->waveType = SQUARE;
			}
			else if(message -> waveType == 2)
			{
				stepsNumber = 100000/message->freq;
				for(i=0;i<=stepsNumber;i++)
				{	
					pwmMsg->steps[i] = message->amplitude/4 + message->amplitude/4*sin(2*PI*message->freq*i*0.00001);
					displayMsg->steps[i] = pwmMsg->steps[i];
				}
				displayMsg->waveType = SINE;
			}
			else if(message -> waveType == 3)
			{
				stepsNumber = 100000/message->freq;
				for(i=0;i<=stepsNumber;i++)
				{	
					pwmMsg->steps[i] = message->amplitude/2 + (message->amplitude/PI)*asin(sin(2*PI*message->freq*0.00001*i));
					displayMsg->steps[i] = pwmMsg->steps[i];
				}
				displayMsg->waveType = TRIANGLE;
			}
			else if(message -> waveType == 4)
			{
				stepsNumber = 100000/message->freq;
				for(i=0;i<=stepsNumber;i++)
				{	
					pwmMsg->steps[i] = (message->amplitude/2 + (-message->amplitude/PI)*atan(1/tan(i*0.00001*PI*message->freq)));
					displayMsg->steps[i] = pwmMsg->steps[i];
				}
				displayMsg->waveType = SAWTOOTH;
			}
			else if(message -> waveType == 5)
			{
				int amp=message->amplitude/2, m = 1/(message->freq);				
				stepsNumber = 100000/message->freq;				
				for(i=0;i<=stepsNumber;i++)
				{	
					pwmMsg->steps[i] = 0;
					for(n=1;n<10;n++){
						pwmMsg->steps[i] += (((8*amp)/(PI*PI))*((sin(n*PI/4)+sin(3*n*PI/4))/(n*n)))*(sin(2*message->freq*PI*0.00001*i*n));
					}
					displayMsg->steps[i] = pwmMsg->steps[i];
				}
				displayMsg->waveType = TRAPEZOID;
			}
			
			pwmMsg->amplitude = message->amplitude;
			pwmMsg->freq = message->freq;
			osMailFree(math_mail_q_id, message);
			
			i = 0;
			while(i<4000 && pwmMsg->steps[i] <500)
			{
				if(pwmMsg->steps[i]>maior)
					maior = pwmMsg->steps[i];
				i++;
			}
			
			i=0;
			while(i<4000 && pwmMsg->steps[i] <500)
			{
				pwmMsg->steps[i] = pwmMsg->steps[i]*pwmMsg->amplitude/maior;
				i++;
			}
			
			if(pwmMsg){
				osMailPut(pwm_mail_q_id,pwmMsg); 
			}		
			if(displayMsg){
				osMailPut(display_mail_q_id,displayMsg); 
			}
		}
	}
}
osThreadDef(thread_math, osPriorityNormal, 1, 0); 

void thread_display(void const *argument){
	osEvent evt;
	display_msg* d_msg;
	char pbuf[10];

	GrStringDraw(&sContext,"Frequencia: ", -1, 0, (sContext.psFont->ui8Height+2)*10, true);
	GrStringDraw(&sContext,"Amplitude: ", -1, 0, (sContext.psFont->ui8Height+2)*11, true);

	while(1) {
		evt = osMailGet(display_mail_q_id, osWaitForever);
		if(evt.status == osEventMail) {
			d_msg = (display_msg*) evt.value.p;
			
			// Write frequency
			floatToString(d_msg->freq, pbuf, 10, 10, 0, 1);
			GrStringDraw(&sContext, pbuf, -1, (sContext.psFont->ui8MaxWidth+2)*12, (sContext.psFont->ui8Height+2)*10, true);

			// Write Amplitude
			floatToString(d_msg->amplitude, pbuf, 10, 10, 0, 2);
			GrStringDraw(&sContext, pbuf, -1, (sContext.psFont->ui8MaxWidth+2)*12, (sContext.psFont->ui8Height+2)*11, true);

			clearDisplayWave();
			printWave(d_msg->waveType);
			
			osMailFree(display_mail_q_id, d_msg);
		}
	}
}
osThreadDef(thread_display, osPriorityNormal, 1, 0); 


void thread_pwm(void const *argument){
	osEvent evt;
	uint32_t lastTick = osKernelSysTick();
	pwm_msg *message = 0;
	int i = 0, aux;
	
	while(1){
		evt = osMailGet(pwm_mail_q_id,0);
		if(evt.status == osEventMail){
			message = (pwm_msg*) evt.value.p;			
			osMailFree(pwm_mail_q_id, message);
			
			// Descarga do filtro
			pwmAmplitudeSet(0.0);
			osDelay(1000);
		}
		if(osKernelSysTick()- lastTick > 400)
		{
			pwmAmplitudeSet(message->steps[i] / 3.3);
			i++;
			lastTick = osKernelSysTick();			
			if(i == 4000 || message->steps[i] > 500)
				i = 0;
		}
 }
}
osThreadDef(thread_pwm, osPriorityNormal, 1, 0);




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
	//Timer_Init();
	pwmInit();
	
	//Create the mutex
	mutex_display = osMutexCreate(osMutex(mutex_display));
	
	
	uart_mail_q_id = osMailCreate(osMailQ(uart_mail_q),NULL);
	math_mail_q_id = osMailCreate(osMailQ(math_mail_q),NULL);
	display_mail_q_id = osMailCreate(osMailQ(display_mail_q),NULL);
	pwm_mail_q_id = osMailCreate(osMailQ(pwm_mail_q),NULL);
	
	// Create the threads
	thread_uart_id 		 = osThreadCreate(osThread(thread_uart), NULL);
	thread_math_id  	 = osThreadCreate(osThread(thread_math), NULL);
	thread_display_id  = osThreadCreate(osThread(thread_display), NULL);
	thread_pwm_id      = osThreadCreate(osThread(thread_pwm), NULL);
	
	// Start the kernel
	osKernelStart();	
	osDelay(osWaitForever);
}