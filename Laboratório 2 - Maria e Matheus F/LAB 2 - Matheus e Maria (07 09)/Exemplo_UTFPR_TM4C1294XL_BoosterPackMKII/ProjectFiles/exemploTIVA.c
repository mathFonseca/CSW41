
/*============================================================================
 *                    Exemplos de utilização do Kit
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

#include "rgb.h"
#include "cfaf128x128x16.h"
#include "servo.h"
#include "temp.h"
#include "opt.h"
#include "buttons.h"
#include "buzzer.h"
#include "joy.h"
#include "mic.h"
#include "accel.h"
#include "led.h"
#include "mensagem.h"

#define LED_A      0
#define LED_B      1
#define LED_C      2
#define LED_D      3
#define LED_CLK    7
#define MSG_LENGTH 68 
//To print on the screen
tContext sContext;
//Systick
void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);

bool f_keyGenerator = false;			// Alterada para TRUE quando t_keyGenerator termina de decodificar a mensagem.
bool f_decodedMessage = false;		// Alterada para TRUE quando t_messageDecode termina de decodificar a mensagem.
bool f_beforePrimeTest = false;		// Alterada para TRUE quando t_lastWordTeste termina de testar a chave
bool f_afterPrimeTest = false;		// Alterada para TRUE quando t_penultimateWordTeste termina de testar a chave

uint32_t key = 3;
uint32_t lastKey = 2;
uint32_t nextKey = 5;

char msg[MSG_LENGTH/2 - 2], pbuf[10];

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
/*----------------------------------------------------------------------------
 *  Transforming float to string
 *---------------------------------------------------------------------------*/
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

/*----------------------------------------------------------------------------
 *    Initializations
 *---------------------------------------------------------------------------*/
void init_display(){
	uint8_t i;
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);

}
void init_all(){
	cfaf128x128x16Init();
	joy_init();
	accel_init();
	buzzer_init(); 
	button_init();
	mic_init();
	rgb_init();
	servo_init();
	temp_init();
	opt_init();
	led_init();
	SysTick_Init();
	init_display();
}

void draw_circle(uint16_t x, uint16_t y){
	GrCircleDraw(&sContext, 
		(sContext.psFont->ui8MaxWidth)*x + (sContext.psFont->ui8MaxWidth)/2, 
		(sContext.psFont->ui8Height+2)*y + sContext.psFont->ui8Height/2 - 1, 
		(sContext.psFont->ui8MaxWidth)/2);
}

void fill_circle(uint16_t x, uint16_t y){
	GrCircleFill(&sContext, 
		(sContext.psFont->ui8MaxWidth)*x + sContext.psFont->ui8MaxWidth/2, 
		(sContext.psFont->ui8Height+2)*y + sContext.psFont->ui8Height/2 - 1, 
		(sContext.psFont->ui8MaxWidth)/2-1);
}

void clear_screen() {
	uint8_t i, j;
	GrContextForegroundSet(&sContext, ClrBlack);
	for(i = 0; i < 128; i++) {
		for(j = 0; j < 128; j++) {
				GrPixelDraw(&sContext, j, i);
		}
	}
	GrContextForegroundSet(&sContext, ClrWhite);
}

uint32_t saturate(uint8_t r, uint8_t g, uint8_t b){
	uint8_t *max = &r, 
					*mid = &g, 
					*min = &b,
					*aux, 
					div, num;
	if (*mid > *max){ aux = max; max = mid; mid = aux; }
	if (*min > *mid){ aux = mid; mid = min; min = aux; }
	if (*mid > *max){	aux = max; max = mid; mid = aux; }
	if(*max != *min){
		div = *max-*min;
		num = *mid-*min;
		*max = 0xFF;
		*min = 0x00;
		*mid = (uint8_t) num*0xFF/div;
	}
	return 	(((uint32_t) r) << 16) | 
					(((uint32_t) g) <<  8) | 
					( (uint32_t) b       );
}
/*----------------------------------------------------------------------------
 *    Threads
 *---------------------------------------------------------------------------*/
void t_keyGenerator(void const *argument){
	// Executa após messagePrint termina de executar.
	if(f_keyGenerator == false)
	{
		
		// Da yield quando termina	
		f_keyGenerator = true;
	}
	else
	{
		// Da yield se não pode executar;
	}
}


void t_messageDecode(void const *argument){
	if(f_keyGenerator == true && f_decodedMessage == false)
	{
		// Decodifica a mensagem
		uint16_t i=0;
		uint16_t par=0;	// Idenficar índice da mensagem (Par / Ímpar)
		uint16_t m_par = (key - lastKey);	// Indices pares: Chave  - Primo Anterior
		uint16_t m_impar = (key - nextKey);	// Indices ímpares: Chave - Próximo Primo
		uint32_t decodedMsg[MSG_LENGTH/2];	// 68 posições. Cada letra ocupa 2 espaços, 34 letras ao todo.
		
		for(i = 0; i < MSG_LENGTH; i += 2)	// For que decodifica a mensagem
		{
			uint32_t word;	// Porque a declaração é dentro do for? Realmente necessário?
			word = men1[i];
			word += men1[i+1] * 16 * 16;
			
			if(par % 2 ==0)	// Se índice par, divide por m_par
			{	
				word /= m_par;
			}
			else	// Se índice ímpar, divide por m_impar
			{	
				/* Necessário tratar overflow aqui*/
				word = 0x0020;	// ASCII 0x20 é Space
			}											
			par++;
			decodedMsg[i/2] = word;
		}
		
		for(i = 0; i < MSG_LENGTH/2 - 2; i++)	// Transfere a mensagem decodificada para vetor mensagem 
		{
			msg[i] = (char)(decodedMsg[i]);
		}			
	
		f_decodedMessage = true;
		// Da yield quando termina
	}
	else
	{
		// Da yield se não pode executar;
	}
}


void t_lastWordTest(void const *argument){
	if(f_keyGenerator == true && f_afterPrimeTest == false)
	{
		// Testa a chave
		// Da yield quando termina	
		f_afterPrimeTest = true;
	}
	else
	{
		// Da yield se não pode executar;
	}
}


void t_penultimateWordTest(void const *argument){
	if(f_keyGenerator == true && f_beforePrimeTest == false)
	{
		// Testa a chave
		// Da yield quando termina	
		f_beforePrimeTest = true;
	}
	else
	{
		// Da yield se não pode executar;
	}
}

void t_messagePrint(void const *argument){
	// Checa se as threads necessárias podem executar ou não.
	if( f_decodedMessage == true && f_beforePrimeTest == true && f_afterPrimeTest == true && f_keyGenerator == true)
	{
		// Imprime a mensagem na tela
		uint16_t i = 0;
		uint16_t displayLines = (MSG_LENGTH/2)/21 + 1;
		for(i = 0; i < displayLines; i++)					// Imprime a mensagem  
		{
			GrStringDraw(&sContext, msg + (i*21), 21, 0, (sContext.psFont->ui8Height+2)*i, true);
		}
		// Ao terminar, reseta as flags para false e dá yield.
		
		f_decodedMessage = false;
		f_beforePrimeTest = false;
		f_afterPrimeTest = false;
		f_keyGenerator  = false;
		
		
	}
	else
	{
		// Não pode executar ainda. Da yield
	}
}


bool isPrime(uint32_t next){
	uint16_t i;
	bool divide=false;
	
	if(next % 2 == 0) 
			divide = true;
			
	for(i = 3; (i*i) < next && !divide; i += 2) {
		if(next % i == 0) {
			divide = true;
			break;
		}
	}
	return !divide; //se divide for false, eh um primo
}

void findNextPrime(){
	uint8_t i=0;
	bool achou = false;
	bool divide = false;
	uint32_t next=0;
	
	lastKey = key;
	key = nextKey;
	next = nextKey+1;
	
	while(!achou){	//find next prime
		if(!isPrime(next)){
			next++;
		}
		else{
			achou = true;
		}
	}
	nextKey = next;
}
void findKey(){
	//testa as duas condicoes	
	char pbufa[10],pbufb[10],pbufc[10],pbufd[10],pbufe[10];
	
	bool achou = false;
	
	uint32_t ultima = men1[MSG_LENGTH-4];
	uint32_t p_ultima = men1[MSG_LENGTH-8];

	ultima+= men1[MSG_LENGTH-3] * 16 * 16;
	ultima+= men1[MSG_LENGTH-2] * 16 * 16 * 16 * 16;
	ultima+= men1[MSG_LENGTH-1] * 16 * 16 * 16 * 16 * 16 * 16;

	p_ultima+= men1[MSG_LENGTH-7] * 16 * 16;
	p_ultima+= men1[MSG_LENGTH-6] * 16 * 16 * 16 * 16;
	p_ultima+= men1[MSG_LENGTH-5] * 16 * 16 * 16 * 16 * 16 * 16;
	
	intToString(ultima, pbufd, 10, 10, 3);
	intToString(p_ultima, pbufe, 10, 10, 3);
	
	while(!achou){
			if(key*lastKey == ultima){
					if(key*nextKey == p_ultima){
							achou =true;
					}
			}
			if(!achou){
					findNextPrime();
			}
			
			intToString(key, pbufa, 10, 10, 3);
			intToString(lastKey, pbufb, 10, 10, 3);
			intToString(nextKey, pbufc, 10, 10, 3);
			
			GrStringDraw(&sContext, "Chave: ", -1, 0, (sContext.psFont->ui8Height+2)*5, true);
			GrStringDraw(&sContext, (char*)pbufa, -1, (sContext.psFont->ui8MaxWidth)*7, (sContext.psFont->ui8Height+2)*5, true);
			
			GrStringDraw(&sContext, "antes: ", -1, 0, (sContext.psFont->ui8Height+2)*6, true);
			GrStringDraw(&sContext, (char*)pbufb, -1, (sContext.psFont->ui8MaxWidth)*7, (sContext.psFont->ui8Height+2)*6, true);
			
			GrStringDraw(&sContext, "proxi: ", -1, 0, (sContext.psFont->ui8Height+2)*7, true);
			GrStringDraw(&sContext, (char*)pbufc, -1, (sContext.psFont->ui8MaxWidth)*7, (sContext.psFont->ui8Height+2)*7, true);
	}
}


int showMessage (void) {
	uint16_t i=0;
	uint16_t par=0;														// Idenficar índice da mensagem (Par / Ímpar)
	uint16_t m_par = (key - lastKey);					// Indices pares: Chave  - Primo Anterior
	uint16_t m_impar = (key - nextKey);				// Indices ímpares: Chave - Próximo Primo
	uint32_t decodedMsg[MSG_LENGTH/2];				// Não entendi pq /2
	int displayLines = (MSG_LENGTH/2)/21 + 1;
  //Initializing all peripherals
	init_all();
	
	for(i = 0; i < MSG_LENGTH; i += 2)				// For que decodifica a mensagem
	{
		uint32_t word;
		word = men1[i];
		word += men1[i+1] * 16 * 16;
		
		if(par % 2 ==0) 												// Se índice par, divide por m_par
		{	
			word /= m_par;
		}
		else																		 // Se índice ímpar, divide por m_impar
		{	
			/* Necessário tratar overflow aqui*/
			word = 0x0020;
		}											
		par++;
		decodedMsg[i/2] = word;
	}
	
	for(i = 0; i < MSG_LENGTH/2 - 2; i++) 		// Transfere a mensagem decodificada para vetor mensagem 
	{
		msg[i] = (char)(decodedMsg[i]);
	}			
	
	for(i = 0; i < displayLines; i++)					// Imprime a mensagem  
	{
		GrStringDraw(&sContext, msg + (i*21), 21, 0, (sContext.psFont->ui8Height+2)*i, true);
	}

}

int main2(){
		init_all();
		findKey();
		showMessage();
	  osDelay(osWaitForever);
		return 0;
}

// Threads
osThreadDef(t_keyGenerator, osPriorityNormal, 1, 0); 
osThreadDef(t_messageDecode, osPriorityNormal, 1, 0); 
osThreadDef(t_lastWordTest, osPriorityNormal, 1, 0); 
osThreadDef(t_penultimateWordTest, osPriorityNormal, 1, 0); 
osThreadDef(t_messagePrint, osPriorityNormal, 1, 0); 

int main(){
	
  //Initializing all peripherals
	init_all();

	osKernelInitialize();
	
	// Cria as Thread
	
	osThreadCreate(osThread(t_keyGenerator), NULL);
	osThreadCreate(osThread(t_messageDecode), NULL);
	osThreadCreate(osThread(t_lastWordTest), NULL);
	osThreadCreate(osThread(t_penultimateWordTest), NULL);
	osThreadCreate(osThread(t_messagePrint), NULL);
	
	osKernelStart(); 
	osDelay(osWaitForever);
	
	return 0;
}

int main1 (void) {

  char pbufx[10], pbufy[10], pbufz[10];
	bool center;
	float temp,lux;
	float mic;
	bool s1_press, s2_press;
	uint8_t  	r, g, b;
	uint32_t color;
	uint16_t x, y, z, angle=0;
	
	//Initializing all peripherals
	init_all();
	//Sidelong menu creation
	//init_sidelong_menu();

  while(1){
		
/*  Acelerometro		*/
			x = accel_read_x();
			y = accel_read_y();
			z = accel_read_z();
			
			floatToString((x*3.30/0xFFF), pbufx, 10, 10, 1, 2);
			floatToString((y*3.30/0xFFF), pbufy, 10, 10, 1, 2);		
			floatToString((z*3.30/0xFFF), pbufz, 10, 10, 1, 2);
					
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrMagenta);
			GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*3, true);
			GrContextForegroundSet(&sContext, ClrYellow);
			GrStringDraw(&sContext,(char*)pbufy, -1, (sContext.psFont->ui8MaxWidth)*11, (sContext.psFont->ui8Height+2)*3, true);
			GrContextForegroundSet(&sContext, ClrCyan);
			GrStringDraw(&sContext,(char*)pbufz, -1, (sContext.psFont->ui8MaxWidth)*16, (sContext.psFont->ui8Height+2)*3, true);

/*	RGB 	*/
			r = x*0xFF/0xFFF;   		// dados do acelerometro
			g = y*0xFF/0xFFF;
			b = z*0xFF/0xFFF;
		
			intToString(r, pbufx, 10, 10, 3);
			intToString(g, pbufy, 10, 10, 3);
			intToString(b, pbufz, 10, 10, 3);
			
			color = saturate(r,g,b);
			color = rgb_color_intensity(color, 0.4);
			rgb_write_color(color);

   		GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrRed);
			GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*2, true);
			GrContextForegroundSet(&sContext, ClrGreen);
			GrStringDraw(&sContext,(char*)pbufy, -1, (sContext.psFont->ui8MaxWidth)*10, (sContext.psFont->ui8Height+2)*2, true);
			GrContextForegroundSet(&sContext, ClrBlue);
			GrStringDraw(&sContext,(char*)pbufz, -1, (sContext.psFont->ui8MaxWidth)*14, (sContext.psFont->ui8Height+2)*2, true);

/*  Joystick		*/		
			x = joy_read_x();
			y = joy_read_y();
			center = joy_read_center();

			intToString(x*200/0xFFF-100, pbufx, 10, 10, 4);
			intToString(y*200/0xFFF-100, pbufy, 10, 10, 4);
			
			if (center)
				intToString(1, pbufz, 10, 10, 1);
			else
				intToString(0, pbufz, 10, 10, 1);
				
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*7, true);
			GrStringDraw(&sContext,(char*)pbufy, -1,  (sContext.psFont->ui8MaxWidth)*11, (sContext.psFont->ui8Height+2)*7, true);
			GrStringDraw(&sContext,(char*)pbufz, -1,  (sContext.psFont->ui8MaxWidth)*18, (sContext.psFont->ui8Height+2)*7, true);
			
/*  Temperatura		*/		
			temp = temp_read_celsius();
			floatToString(temp, pbufx, 10, 10, 2, 3);
		
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*4, true);

/*	Light			*/
	
			lux = opt_fread_lux();
		
			floatToString(lux, pbufx, 10, 10, 4, 3);
		
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*5, true);
	
/*	Microfone 	*/
			mic = mic_norm();
		
			intToString((int32_t) (mic*200-100), pbufx, 10, 10, 4);

			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*6, true);

/*	Botoes 	*/			
			s1_press = button_read_s1();
			s2_press = button_read_s2();

			if (s1_press)
				intToString(1, pbufx, 10, 10, 1);
			else
				intToString(0, pbufx, 10, 10, 1);

			if (s2_press)
				intToString(1, pbufy, 10, 10, 1);
			else
				intToString(0, pbufy, 10, 10, 1);

			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*8, true);
			GrStringDraw(&sContext,(char*)pbufy, -1,  (sContext.psFont->ui8MaxWidth)*11, (sContext.psFont->ui8Height+2)*8, true);
			
	}	
}

