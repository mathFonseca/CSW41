
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
#define MENSAGEM 1

//To print on the screen
tContext sContext;
//Systick
void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);
uint32_t startTick;

bool f_keyGenerator = false;			// Alterada para TRUE quando t_keyGenerator termina de decodificar a mensagem.
bool f_decodedMessage = false;		// Alterada para TRUE quando t_messageDecode termina de decodificar a mensagem.
bool f_beforePrimeTest = false;		// Alterada para TRUE quando t_lastWordTeste termina de testar a chave
bool f_afterPrimeTest = false;		// Alterada para TRUE quando t_penultimateWordTeste termina de testar a chave

bool t_ultimoPrimo = false;				// Alterada para TRUE quando o teste do ultimo primo der certo
bool t_penultimoPrimo = false;		// Alterada para TRUE quando o teste do ultimo primo der certo

uint32_t key = 3;
uint32_t lastKey = 2;
uint32_t nextKey = 5;

uint8_t men[MSG_LENGTH];

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
void InitializeMessege(){
	uint8_t i;
	
	for(i=0; i<MSG_LENGTH; i++){
		if(MENSAGEM==1)
			men[i] = men1[i];
		else if(MENSAGEM==2)
			men[i] = men2[i];
		else
			men[i] = men3[i];
	}

}
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
bool isPrime(uint32_t next){
	uint16_t i;
	bool divide=false;
	
	if(next==0 || next==1)
			divide = true;
	
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
void findFirstKey(){

	bool achou = false;
	
	uint16_t find_limit =0;
	uint16_t	next_key = 0;
	
	uint32_t ultima = men[MSG_LENGTH-4];
	ultima+= men[MSG_LENGTH-3] * 16 * 16;
	ultima+= men[MSG_LENGTH-2] * 16 * 16 * 16 * 16;
	ultima+= men[MSG_LENGTH-1] * 16 * 16 * 16 * 16 * 16 * 16;

	while(!achou){
		if(find_limit*find_limit>=ultima){
			achou = true;
		}
		else{
			next_key = find_limit;
			find_limit+=1000;
		}
	}
	//chave tá entre find_key e find_limit
	//achar os primeiros tres primos para salvar
	//acha "lastKey"
	next_key++;
	achou = false;
	while(!achou)
	{	
		if(!isPrime(next_key)){
			next_key++;
		}
		else{
			achou = true;
		}
	}
	lastKey = next_key;
	//acha "key"
	next_key++;
	achou = false;
	while(!achou)
	{	
		if(!isPrime(next_key)){
			next_key++;
		}
		else{
			achou = true;
		}
	}
	key = next_key;
	//acha "nextKey"
	next_key++;
	achou = false;
	while(!achou)
	{	
		if(!isPrime(next_key)){
			next_key++;
		}
		else{
			achou = true;
		}
	}
	nextKey = next_key;
}
void t_keyGenerator(void const *argument){
	while(true){
		// Executa após messagePrint termina de executar.
		if(f_keyGenerator == false)
		{
			// Procura um primo
			uint8_t i=0;
			bool achou = false;
			bool divide = false;
			uint32_t next=0;
			
			lastKey = key;
			key = nextKey;
			next = nextKey+1;
			
			while(!achou)
			{	//find next prime
				
				// Checa se é primo pela func isPrime(next)			
				if(!isPrime(next))
				{
					next++;
				}
				else{
					achou = true;
				}
			}
			nextKey = next;
			
			//chave 13399
			//anterior 48337
			//proxima 48353
			
			// Da yield quando termina	
			f_keyGenerator = true;
			osThreadYield();
		}
		else
		{
			// Da yield se não pode executar;
			osThreadYield();
		}
	}
}


void t_messageDecode(void const *argument){
	while(true){
			if(f_keyGenerator == true && f_decodedMessage == false)
			{
				// Decodifica a mensagem
				uint16_t i=0;
				uint16_t par=0;	// Idenficar índice da mensagem (Par / Ímpar)
				uint16_t m_par = (key - lastKey);	// Indices pares: Chave  - Primo Anterior
				uint8_t m_impar = nextKey - key;	// Indices ímpares: Chave - Próximo Primo
				uint32_t decodedMsg[MSG_LENGTH/2];	// 68 posições. Cada letra ocupa 2 espaços, 34 letras ao todo.
				
				for(i = 0; i < MSG_LENGTH; i += 2)	// For que decodifica a mensagem
				{
					int16_t word;	// Porque a declaração é dentro do for? Realmente necessário?
					word = men[i];
					word += men[i+1] * 16 * 16;
					
					if(par % 2 ==0)	// Se índice par, divide por m_par
					{	
						word /= m_par;
					}
					else	// Se índice ímpar, divide por m_impar
					{	
						/* Necessário tratar overflow aqui*/
						word = (word/(-m_impar));
						//word = word % 0xff;
						//word = 0x20;
					}											
					par++;
					decodedMsg[i/2] = word;
				}
				
				for(i = 0; i < MSG_LENGTH/2 - 4; i++)	// Transfere a mensagem decodificada para vetor mensagem 
				{
					msg[i] = (char)(decodedMsg[i]);
				}			
			
				f_decodedMessage = true;
				// Da yield quando termina
				osThreadYield();
			}
			else
			{
				// Da yield se não pode executar;
				osThreadYield();
			}
	}
}

void t_lastWordTest(void const *argument){
	while(!t_ultimoPrimo){
		if(f_keyGenerator == true && f_afterPrimeTest == false)
		{
			// Testa a chave
			char pbufa[10],pbufb[10],pbufc[10],pbufd[10],pbufe[10];
			
			uint32_t ultima = men[MSG_LENGTH-4];
			
			ultima+= men[MSG_LENGTH-3] * 16 * 16;
			ultima+= men[MSG_LENGTH-2] * 16 * 16 * 16 * 16;
			ultima+= men[MSG_LENGTH-1] * 16 * 16 * 16 * 16 * 16 * 16;

			if(key*lastKey == ultima)
					t_ultimoPrimo =true;	
			// Da yield quando termina	
			f_afterPrimeTest = true;
			osThreadYield();
		}
		else
		{
			// Da yield se não pode executar;
			osThreadYield();
		}
	}
}


void t_penultimateWordTest(void const *argument){
	while(!t_penultimoPrimo){
		if(f_keyGenerator == true && f_beforePrimeTest == false)
		{
			// Testa a chave
			char pbufa[10];
		
			uint32_t p_ultima = men[MSG_LENGTH-8];
			p_ultima+= men[MSG_LENGTH-7] * 16 * 16;
			p_ultima+= men[MSG_LENGTH-6] * 16 * 16 * 16 * 16;
			p_ultima+= men[MSG_LENGTH-5] * 16 * 16 * 16 * 16 * 16 * 16;
			
			if(key*nextKey == p_ultima){
					t_penultimoPrimo =true;
			}
			
			intToString(key, pbufa, 10, 10, 3);
			
			GrStringDraw(&sContext, "Chave: ", -1, 0, (sContext.psFont->ui8Height+2)*5, true);
			GrStringDraw(&sContext, (char*)pbufa, -1, (sContext.psFont->ui8MaxWidth)*7, (sContext.psFont->ui8Height+2)*5, true);
			
			// Da yield quando termina

			f_beforePrimeTest = true;
			osThreadYield();			
		}
		else
		{
			osThreadYield();
			// Da yield se não pode executar;
		}
	}
}

void t_messagePrint(void const *argument){
	while(true){
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
			
			if(t_penultimoPrimo && t_ultimoPrimo){
				char pbuf[10];
				float time = ((osKernelSysTick() - startTick) / (float)osKernelSysTickFrequency) * 145.0;
				floatToString(time, pbuf, 10, 10, 0, 3);
				//GrStringDraw(&sContext, "Chave encontrada!", -1, 0, (sContext.psFont->ui8Height+2)*6, true);
				GrStringDraw(&sContext, "Tempo (ms): ", -1, 0, (sContext.psFont->ui8Height+2)*7, true);
				GrStringDraw(&sContext, (char*)pbuf, -1, (sContext.psFont->ui8MaxWidth)*12, (sContext.psFont->ui8Height+2)*7, true);
			
			}
			
			
			// Ao terminar, reseta as flags para false e dá yield.
			
			f_decodedMessage = false;
			f_beforePrimeTest = false;
			f_afterPrimeTest = false;
			f_keyGenerator  = false;
			
			osThreadYield();
			
		}
		else
		{
			// Não pode executar ainda. Da yield
			osThreadYield();
		}
	}
}



// Threads
osThreadDef(t_keyGenerator, osPriorityNormal, 1, 0); 
osThreadDef(t_messageDecode, osPriorityNormal, 1, 0); 
osThreadDef(t_lastWordTest, osPriorityNormal, 1, 0); 
osThreadDef(t_penultimateWordTest, osPriorityNormal, 1, 0); 
osThreadDef(t_messagePrint, osPriorityNormal, 1, 0); 

int main(){
	osKernelInitialize();
  //Initializing all peripherals
	init_all();
		
	// Cria as Thread
	startTick = osKernelSysTick();
	InitializeMessege();
	findFirstKey();
	osThreadCreate(osThread(t_keyGenerator), NULL);
	osThreadCreate(osThread(t_messageDecode), NULL);
	osThreadCreate(osThread(t_lastWordTest), NULL);
	osThreadCreate(osThread(t_penultimateWordTest), NULL);
	osThreadCreate(osThread(t_messagePrint), NULL);
	
	osKernelStart(); 

}
