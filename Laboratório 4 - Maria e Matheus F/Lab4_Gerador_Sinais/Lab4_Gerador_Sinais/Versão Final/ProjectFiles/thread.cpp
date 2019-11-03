#include "cmsis_os.h"
#include <stdio.h>
#include <stdlib.h>
#include "TM4C129.h" // Device header
#include <stdbool.h>
#include "grlib/grlib.h"
#include "cfaf128x128x16.h"
#include "led.h"
#include "joy.h"
#include "buttons.h"
#include "driverlib/sysctl.h" // driverlib
#include <string.h>
#include "gerencia_pilulas.h"

//#define GANTT 1

/*----MAPA----*/
// Treshold de cor dos pixels
#define TRESHOLD 128
#define FPS 50
#define LINHA_LENGTH 67
#define COLUNA_LENGTH 128

/*----FANTASMA----*/
// Tamanho fantasma
#define FANTASMA_LINHA_LENGTH 7
#define FANTASMA_COLUNA_LENGTH 7

/*----PACMAN----*/
// Tamanho pacman
#define PACMAN_LINHA_LENGTH 7
#define PACMAN_COLUNA_LENGTH 7
#define X_INICIAL_PACMAN 60
#define Y_INICIAL_PACMAN 50

uint32_t pos_x_pacman = X_INICIAL_PACMAN;
uint32_t pos_y_pacman = Y_INICIAL_PACMAN;
uint32_t pos_x_pacman_teste,pos_y_pacman_teste;
uint8_t dir_joy=5; // 0 - cima,1 - direita, 2-baixo, 3-esquerda

// Variaveis timer
osTimerId frames_ID;


// Variaveis das imagens
// MAPA
extern const unsigned char mapa[];
extern const unsigned char mapa_length;	
// FANTASMA
extern const unsigned char fantasma1[];
extern const unsigned char fantasma1_length;
// PACMAN
extern const unsigned char pacman1[];
extern const unsigned char pacman1_length;

// Função inicia periféricos	
void init_all(){
	cfaf128x128x16Init();
	led_init();
	joy_init();
	button_init();
}

// Variáveis do display
tContext sContext;
/*----------------------------------------------------------------------------
 *  Funçoes implementadas
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




bool verifica_colisao_pacman(uint32_t pos_x,uint32_t pos_y){

	// Localiza o pacman na memoria
	uint32_t pos_teste =(pos_x-1)+COLUNA_LENGTH*pos_y;
	
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrPixelDraw(&sContext,pos_x,pos_y);//SUPERIOR ESQUERDO
	GrPixelDraw(&sContext,pos_x+PACMAN_LINHA_LENGTH+1,pos_y);//INFERIOR ESQUERDO
	GrPixelDraw(&sContext,pos_x,pos_y+PACMAN_COLUNA_LENGTH+1);//SUPERIOR DIREITO
	GrPixelDraw(&sContext,pos_x+PACMAN_LINHA_LENGTH+1,pos_y+PACMAN_LINHA_LENGTH+1); //INFERIOR DIREITO
	
	
	
	// Se o pacman deseja ir pra cima, verifica possibilidade
	// Se for menor que o treshold, significa que estou batendo num ponto preto
	if(dir_joy == 0 && (mapa[pos_teste] < TRESHOLD || mapa[pos_teste+PACMAN_COLUNA_LENGTH+1] < TRESHOLD))
		return false;
	else
	//Testa movimentacao para a direita	
	if(dir_joy == 1 && mapa[pos_teste+PACMAN_COLUNA_LENGTH] < TRESHOLD && mapa[pos_teste+PACMAN_COLUNA_LENGTH+PACMAN_LINHA_LENGTH*COLUNA_LENGTH] < TRESHOLD)
		return false;
	else
	//Testa movimentacao para baixo
	if(dir_joy == 2 && mapa[pos_teste+(PACMAN_LINHA_LENGTH-1)*COLUNA_LENGTH] < TRESHOLD && mapa[pos_teste+(PACMAN_COLUNA_LENGTH-1)+(PACMAN_LINHA_LENGTH-1)*COLUNA_LENGTH] < TRESHOLD)
		return false;
	else
	//Testa movimentacao para esquerda
	if(dir_joy == 3 && mapa[pos_teste] < TRESHOLD && mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH] < TRESHOLD)
		return false;
	
	// Se falhar em todos os casos, confirma a possibilidade de movimentacao
		return true;
}




// Criação das threads 
void Menu_inicial(void const *argument);
void Desenho_inicial(void const *argument);
void Gerencia_botoes(void const *argument);
void Fantasma(void const *argument);
void Pacman(void const *argument);
void Gerencia_pilulas(void const *argument);
void Gerencia_vitaminas(void const *argument);
void Painel_instrumentos(void const *argument);
void Atualiza_desenho(void const *argument);

// Variável que determina ID das threads
osThreadId Menu_inicial_ID; 
osThreadId Desenho_inicial_ID; 
osThreadId Gerencia_botoes_ID; 
osThreadId Fantasma_ID; 
osThreadId Pacman_ID;
osThreadId Gerencia_pilulas_ID; 
osThreadId Gerencia_vitaminas_ID; 
osThreadId Painel_instrumentos_ID; 
osThreadId Atualiza_desenho_ID; 

// Dfinição das threads
osThreadDef (Menu_inicial, osPriorityNormal, 1, 0);     // thread object
osThreadDef (Desenho_inicial, osPriorityNormal, 1, 0);     // thread object
osThreadDef (Gerencia_botoes, osPriorityNormal, 1, 0);     // thread object
osThreadDef (Fantasma, osPriorityNormal, 1, 0);     // thread object
osThreadDef (Pacman, osPriorityNormal, 1, 0);     // thread object
osThreadDef (Gerencia_pilulas, osPriorityNormal, 1, 0);     // thread object
osThreadDef (Gerencia_vitaminas, osPriorityNormal, 1, 0);     // thread object
osThreadDef (Painel_instrumentos, osPriorityNormal, 1, 0);     // thread object
osThreadDef (Atualiza_desenho, osPriorityNormal, 1, 0);     // thread object

// Inicializa as threads
int Init_Thread (void) {

	Menu_inicial_ID = osThreadCreate (osThread(Menu_inicial), NULL);
	if (!Menu_inicial_ID) return(-1);
	
	Desenho_inicial_ID = osThreadCreate (osThread(Desenho_inicial), NULL);
	if (!Desenho_inicial_ID) return(-1);
	
	Gerencia_botoes_ID = osThreadCreate (osThread(Gerencia_botoes), NULL);
	if (!Gerencia_botoes_ID) return(-1);
	
	Fantasma_ID = osThreadCreate (osThread(Fantasma), NULL);
	if (!Fantasma_ID) return(-1);
	
	Pacman_ID = osThreadCreate (osThread(Pacman), NULL); 
	if (!Pacman_ID) return(-1);
	
	Gerencia_pilulas_ID = osThreadCreate (osThread(Gerencia_pilulas), NULL); 
	if (!Gerencia_pilulas_ID) return(-1);
	
	Gerencia_vitaminas_ID = osThreadCreate (osThread(Gerencia_vitaminas), NULL); 
	if (!Gerencia_vitaminas_ID) return(-1);
	
	Painel_instrumentos_ID = osThreadCreate (osThread(Painel_instrumentos), NULL); 
	if (!Painel_instrumentos_ID) return(-1);
 
	Atualiza_desenho_ID = osThreadCreate (osThread(Atualiza_desenho), NULL); 
	if (!Atualiza_desenho_ID) return(-1);
	
	return(0);
}



// Callback do timer
void Timer_fps (void){
	while(true){
	osSignalSet(Gerencia_botoes_ID,0x01);
	//osSignalSet(Fantasma_ID,0x01);	
	}
}

// Thread que gear os números  
void Menu_inicial (void const *argument) {
	while(true){
		osSignalWait(0x01,osWaitForever);
		osSignalSet(Desenho_inicial_ID,0x01);
		// Start do timer
		// osTimerStart(frames_ID,FPS);
	}	
}
// Thread que verifica os primos
void Desenho_inicial (void const *argument){
	uint32_t coluna,linha;
	uint32_t index = 0;

	index = 0;
	// Desenho do mapa no display
	for(linha=1;linha<=LINHA_LENGTH;linha++){
		for(coluna=1;coluna<=COLUNA_LENGTH;coluna++){
			
			#ifndef GANTT
			
			if(mapa[index++]>TRESHOLD)
				GrContextForegroundSet(&sContext, ClrBlack);
			else
				GrContextForegroundSet(&sContext, ClrBlue);
			
			GrPixelDraw(&sContext,coluna,linha);
			
			#endif
		}
	}
	while(true){	
		// Espera a habilitação do menu inicial
		osSignalWait(0x01,osWaitForever);
		osSignalClear(Desenho_inicial_ID,0x01);

	index = 0;
	// Desenho do mapa no display
	for(linha=1;linha<=LINHA_LENGTH;linha++){
		for(coluna=1;coluna<=COLUNA_LENGTH;coluna++){
			
			#ifndef GANTT
			
			if(mapa[index++]>TRESHOLD)
				GrContextForegroundSet(&sContext, ClrBlack);
			else
				GrContextForegroundSet(&sContext, ClrBlue);
			
			GrPixelDraw(&sContext,coluna,linha);
			
			#endif
		}
	}
//		// Desenho de um fantasma no display
//		index = 1;
//		for(linha=1;linha<=FANTASMA_LINHA_LENGTH;linha++){
//			for(coluna=1;coluna<=FANTASMA_COLUNA_LENGTH;coluna++){
//				
//				#ifndef GANTT
//				if(fantasma1[index++]>TRESHOLD)
//					GrContextForegroundSet(&sContext, ClrBlack);
//				else
//					GrContextForegroundSet(&sContext, ClrYellow);
//				
//				GrPixelDraw(&sContext,coluna+3,linha+2);
//				#endif
//			}
//		}

			// Desenho do PACMAN 
		index = 1;
		for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
			for(coluna=1;coluna<=PACMAN_COLUNA_LENGTH;coluna++){
				
				#ifndef GANTT
				if(pacman1[index++]>TRESHOLD)
					GrContextForegroundSet(&sContext, ClrBlack);
				else
					GrContextForegroundSet(&sContext, ClrYellow);
				
				GrPixelDraw(&sContext,coluna+pos_x_pacman,linha+pos_y_pacman);
				
				#endif
			}
		}
		
		// TESTANDO MOVIMENTACAO
		osSignalSet(Gerencia_botoes_ID,0x01);
	}
}

// Thread que decodifica a mensagem
void Gerencia_botoes(void const *argument) {
	int16_t joy_x;
	int16_t joy_y;
	
	while(true){

		// Espera a habilitação do timer de fps
		osSignalWait(0x01,osWaitForever);
		osSignalClear(Gerencia_botoes_ID,0x01);
		
		#ifndef GANTT
		joy_x = joy_read_x();
		joy_y = joy_read_y();
		#endif
		
		joy_x = joy_x*200/0xFFF-100;
		joy_y = joy_y*200/0xFFF-100;
		
		// joystik cima
		if ( joy_y > 90 & joy_x > -20 & joy_x < 20){
			pos_y_pacman_teste = pos_y_pacman - 1;
			pos_x_pacman_teste = pos_x_pacman;
			dir_joy = 0;
		}								
		else
		// joystick baixo
		if ( joy_y < -90 & joy_x > -20 & joy_x < 20){
			pos_y_pacman_teste = pos_y_pacman + 1;
			pos_x_pacman_teste = pos_x_pacman;
			dir_joy = 2;
		}									
		else
		// joystick esquerda
		if ( joy_x < -90 & joy_y > -20 & joy_y < 20){
			pos_y_pacman_teste = pos_y_pacman;
			pos_x_pacman_teste = pos_x_pacman - 1;
			dir_joy = 3;
		}	
		else
		// joystick direita
		if ( joy_x > 80 & joy_y > -20 & joy_y < 20){
			pos_y_pacman_teste = pos_y_pacman;
			pos_x_pacman_teste = pos_x_pacman + 1;		
			dir_joy = 1;
		}
		else{
			pos_y_pacman_teste = pos_y_pacman;
			pos_x_pacman_teste = pos_x_pacman;						
		}
		// Seta o flag do pacman, para verificar possibilidade de movimentacao
		osSignalSet(Pacman_ID,0x01);
	}
}
	
// Thread que verifica o penultimo valor
void Fantasma(void const *argument) {

	while(true){
		// Espera a habilitação do timer de fps
		osSignalWait(0x01,osWaitForever);
	}
}

// Thread que verifica o ultimo valor
void Pacman(void const *argument) {

	while(true){
		// Espera a habilitação do timer de fps
		osSignalWait(0x01,osWaitForever);
		osSignalClear(Pacman_ID,0x01);
		
		// Posicao inicial do pacman
		// Se o teste de colisao der verdadeiro, quer dizer que nao ha colisao
		// Deste modo, podemos trocar as variaveis
		if(verifica_colisao_pacman(pos_x_pacman_teste,pos_y_pacman_teste)){
			pos_x_pacman = pos_x_pacman_teste;
			pos_y_pacman = pos_y_pacman_teste;
		}
		// mantem as variaveis em caso de reprovacao do teste de colisao
		
		
		
		
		
		
		
		// Seta o flag do print
		osSignalSet(Desenho_inicial_ID,0x01);
	}
}
// Thread que mostra o resultado no oled
void Gerencia_pilulas(void const *argument) {
	while(true){
		// Espera a habilitação do timer de fps
		osSignalWait(0x01,osWaitForever);
		osSignalClear(Gerencia_pilulas_ID,0x01);	
	}
}
// Thread que mostra o resultado no oled
void Gerencia_vitaminas(void const *argument) {
	while(true){
		// Espera a habilitação do timer de fps
		osSignalWait(0x01,osWaitForever);
		osSignalClear(Gerencia_pilulas_ID,0x01);	
	}
}
// Thread que finaliza e mostra o tempo de execucao
void Painel_instrumentos(void const *argument) {
	while(true){	
		// Espera a habilitação do timer de fps
		osSignalWait(0x01,osWaitForever);
		osSignalWait(0x02,osWaitForever);
		osSignalWait(0x03,osWaitForever);
		osSignalClear(Painel_instrumentos_ID,0x06);	
	}
}
// Thread que finaliza e mostra o tempo de execucao
void Atualiza_desenho(void const *argument) {
	
	while(true){
		// Espera a habilitação do painel de instrumentos
		osSignalWait(0x01,osWaitForever);
		osSignalClear(Atualiza_desenho_ID,0x01);	
	}
}



// Criação do timer
//osTimerDef(frames,Timer_fps);

int main() {
	
	// Inicializacao do timer
	//frames_ID = osTimerCreate(osTimer(frames),osTimerPeriodic,0);
	
	// Inicializa o Kernel
	osKernelInitialize();	

#ifndef GANTT	
	//Initializing all peripherals
	init_all();

	// Inicializa o Display
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8 );
	GrContextBackgroundSet(&sContext, ClrBlack);
#endif

	// Inicializa as Threads
	if(Init_Thread()==0)
		return 0;
		// Mensagens de erro de inicialização
	
	// Seta o signal do menu inicial
	osSignalSet(Menu_inicial_ID,0x01);
	
	// Inicializa o Kernel, junto com as threads
	osKernelStart();
	
	//Main aguarda para sempre
	osDelay(osWaitForever);

}
