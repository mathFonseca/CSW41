
#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include "grlib/grlib.h"
#include "cfaf128x128x16.h"
#include "buttons.h"
#include <math.h>


#define PI 3.14159265359
#define PI2	9.8696044010
#define TEMPO 25

float resultado_geral;

// To print on the screen
tContext sContext;

void init_all1(){
	cfaf128x128x16Init();
	button_init();
}

void init_display_context1(){
	GrContextInit(&sContext, &g_sCfaf128x128x16);

	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);

	GrContextForegroundSet(&sContext, ClrYellow);
	GrContextBackgroundSet(&sContext, ClrBlack);
}

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

int fatorial(uint32_t x){
	uint8_t i;
	int result;
	result = 1;
	for(i=1; i<=x; i++)
		result *= i;

	return result;
}

// Implementações das Threads
void thread_task_A1() {

	int i;
	uint64_t result;
	
	result = 0;
	for(i=0; i<=256; i++){
			result +=i+(i+2);
	}
	
	resultado_geral = result;
}
//osThreadDef(thread_task_A, osPriorityNormal, 1, 0);

void thread_task_B1() {

		uint8_t i;
		float result;

		result = 0;

		for(i=1; i<=16; i++){
				result += (1<<i)/(fatorial(i));
		}
		resultado_geral = result;
}
//osThreadDef(thread_task_B, osPriorityNormal, 1, 0);

void thread_task_C1() {

	uint8_t i;
	float result;

	result = 0;

	for(i=0; i<=72; i++){
			result += (i+1)/(i);
	}
	resultado_geral = result;
}
//osThreadDef(thread_task_C, osPriorityNormal, 1, 0);

void thread_task_D1() {

	uint8_t i;
	float result;

	result = 0;

	result = 1 + (5/fatorial(3) + (5/(fatorial(5))) + (5/(fatorial(7))) + (5/(fatorial(9))));

	//faz algum tipo de verificação
	resultado_geral = result;
}
//osThreadDef(thread_task_D, osPriorityNormal, 1, 0);

void thread_task_E1() {

	uint8_t i;
	uint64_t result;

	result = 0;
	for(i=1; i<=100; i++){
			result += i * PI2;// (PI^2); // ele ta reclamando de alguma coisa aqui. Acho que deve ser o PI^2
	}
	resultado_geral = result;
}
//osThreadDef(thread_task_E, osPriorityNormal, 1, 0);

void thread_task_F1() {

	uint8_t i;
	uint64_t result;

	result = 0;

	for(i=1; i<=128; i++){
			result += (i^3)/(2^i);
	}
	resultado_geral = result;
}
//osThreadDef(thread_task_F, osPriorityNormal, 1, 0);

int main (void) {
	
	int contadorTicks;
	char pbuf[10];
	char pbuf2[10];
	// Initialize the kernel
	osKernelInitialize();

	//Initializing all peripherals
	init_all1();
	init_display_context1();

	osKernelStart();	
	
	contadorTicks = osKernelSysTick();
	thread_task_D1();
	contadorTicks = osKernelSysTick() - contadorTicks;
	
	
	GrContextForegroundSet(&sContext, ClrRed);

	intToString(contadorTicks, pbuf,  10, 10, 4);
	GrStringDraw(&sContext, "TICKS:", -1,0, (sContext.psFont->ui8Height+2)*1, true);
	GrStringDraw(&sContext, (char*)pbuf, -1, 40, (sContext.psFont->ui8Height+2)*1, true);
	
	
	floatToString(resultado_geral, pbuf2,  10, 10, 4, 4);
	GrStringDraw(&sContext, "RESULT:", -1, 0, (sContext.psFont->ui8Height+2)*3, true);
	GrStringDraw(&sContext, (char*)pbuf2, -1, 40, (sContext.psFont->ui8Height+2)*3, true);
	
	osDelay(osWaitForever);
}
