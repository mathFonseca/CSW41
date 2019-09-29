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
#include "EddieV3.h"
#include "Background.h"

#define LED_A      0
#define LED_B      1
#define LED_C      2
#define LED_D      3
#define LED_CLK    7
#define EDDIE_WIDTH 8
#define EDDIE_HEIGHT 15
#define EDDIE_PASSO 4
#define EDDIE_PASSO_JUMPING 3
#define ESCADA_WIDTH 22
#define ESCADA_HEIGHT 20
#define ESCADA_COUNTER 4
#define INICIO_Y 30


//To print on the screen
tContext sContext;

//escadas
uint32_t escadas_x[4];
uint32_t escadas_y[4];
bool may_go_up;
bool may_go_down;
//Eddie
uint32_t Eddie_x;
uint32_t Eddie_y;
uint32_t Eddie_last_x;
uint32_t Eddie_last_y;
bool going_up;
uint8_t going_up_counter;
bool going_down;
uint8_t going_down_counter;
bool jumping;
uint8_t jumping_counter;
uint8_t jumping_side; //0=parado, 1=direita, 2=esquerda
//bool can_jump;
//game controller
uint8_t level;


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

/*----------------------------------------------------------------------------
 *    Initializations
 *---------------------------------------------------------------------------*/

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
}

void draw_pixel(uint16_t x, uint16_t y){
	GrPixelDraw(&sContext, x, y);
}

void draw_Eddie(uint16_t x, uint16_t y){
	/*
	uint32_t i, j;
	for(i=x; i<x+EDDIE_WIDTH; i++){
		for(j=y; j<y+EDDIE_HEIGHT; j++){
				draw_pixel(i,j);
		}
	}
	}*/
	
	uint32_t i, j;
	uint32_t color, pixel=0;

	for(j=y; j<y+EDDIE_HEIGHT; j++){ //y
			for(i=x; i<x+EDDIE_WIDTH; i++){//x
					pixel = Eddie_start+3*((j-y)*EDDIE_WIDTH+i-x);
					color = 0x00;
					color += Eddie[pixel+2];
					color += Eddie[pixel+1]*16*16;
					color += Eddie[pixel]*16*16*16*16;
					color += 0x00*16*16*16*16*16*16;
					GrContextForegroundSet(&sContext, color);
					if(color!=ClrBlack)
						draw_pixel(i, j);
			}
	}
}

void drawBack(uint16_t x, uint16_t y,uint16_t lenght, uint16_t width){
	uint32_t i, j;
	uint32_t color, pixel;

	for(j=y; j<y+width; j++){ //y
		for(i=x; i<x+lenght; i++){//x
			
			if(j<=INICIO_Y){
					GrContextForegroundSet(&sContext, ClrBlack);
			}
			else{
					pixel = Background_start+3*((j- INICIO_Y)*128+i);
					color = 0x00;
					color += Background[pixel+2];
					color += Background[pixel+1]*16*16;
					color += Background[pixel]*16*16*16*16;
					color += 0x00*16*16*16*16*16*16;
					GrContextForegroundSet(&sContext, color);
					//GrContextForegroundSet(&sContext, ClrBlueViolet);		
			}
			draw_pixel(i, j);
			
		}
	}
}
void drawBackV1(uint16_t x, uint16_t y,uint16_t lastx, uint16_t lasty){
	uint16_t i, j;
	uint32_t color, pixel;
	uint16_t ini_i, ini_j, end_i, end_j;
	
	if(x>lastx) { ini_i=lastx; end_i=x;}
	else				{ ini_i=x; end_i=lastx;}
	if(y>lasty) { ini_j=lasty; end_j=y;}
	else				{ ini_j=y; end_j=lasty;}
	

	for(j=ini_j; j<end_j+1; j++){ //y
		for(i=ini_i; i<end_i+1; i++){//x
			pixel = Background_start+3*((j- INICIO_Y)*128+i);
			color = 0x00;
			color += Background[pixel+2];
			color += Background[pixel+1]*16*16;
			color += Background[pixel]*16*16*16*16;
			color += 0x00*16*16*16*16*16*16;
			GrContextForegroundSet(&sContext, color);
			//GrContextForegroundSet(&sContext, ClrBlueViolet);
			draw_pixel(i, j);
		}
	}
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

void init_sidelong_menu(){
	uint8_t i;
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
	
	//Escreve menu lateral:
	GrStringDraw(&sContext,"Exemplo EK-TM4C1294XL", -1, 0, (sContext.psFont->ui8Height+2)*0, true);
	GrStringDraw(&sContext,"---------------------", -1, 0, (sContext.psFont->ui8Height+2)*1, true);
	GrStringDraw(&sContext,"RGB", -1, 0, (sContext.psFont->ui8Height+2)*2, true);
	GrStringDraw(&sContext,"ACC", -1, 0, (sContext.psFont->ui8Height+2)*3, true);
	GrStringDraw(&sContext,"TMP", -1, 0, (sContext.psFont->ui8Height+2)*4, true);
	GrStringDraw(&sContext,"OPT", -1, 0, (sContext.psFont->ui8Height+2)*5, true);
	GrStringDraw(&sContext,"MIC", -1, 0, (sContext.psFont->ui8Height+2)*6, true);
	GrStringDraw(&sContext,"JOY", -1, 0, (sContext.psFont->ui8Height+2)*7, true);
	GrStringDraw(&sContext,"BUT", -1, 0, (sContext.psFont->ui8Height+2)*8, true);
	GrStringDraw(&sContext,"NOVO ICONE", -1, 0, (sContext.psFont->ui8Height+2)*9, true);

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
 *      Switch LED on
 *---------------------------------------------------------------------------*/
void Switch_On (unsigned char led) {
  if (led != LED_CLK) led_on (led);
}
/*----------------------------------------------------------------------------
 *      Switch LED off
 *---------------------------------------------------------------------------*/
void Switch_Off (unsigned char led) {
  if (led != LED_CLK) led_off (led);
}





	


/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/


bool button_read_debounce(void) {
	uint8_t i = 0;
	while(i < 50) {
		if(button_read_s1())
			i++;
		else
			return false;
	}
	return true;
}
void InitializeBacKGround(){
		uint32_t color = 0x00, i, j, pixel,pixel_ed;
		
		pixel = Background_start;
		for(j = INICIO_Y; j < 128; j++){ //y
			for(i = 0; i < 128; i++){ //x
				//pixel = Background_start+3*((j-INICIO_Y)*128+i);
				color = 0x00;
				color += Background[pixel+2];
				color += Background[pixel+1]*16*16;
				color += Background[pixel]*16*16*16*16;
				color += 0x00*16*16*16*16*16*16;
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i, j);
				pixel += 3;
			}
		}

}

void Initialize(){

		escadas_x[0] = 100;
		escadas_x[1] = 5;
		escadas_x[2] = 80;
		escadas_x[3] = 10;
	
		escadas_y[0] = 35;//103;//+INICIO_Y;
		escadas_y[1] = 58;//80;//+INICIO_Y;	
		escadas_y[2] = 80;//+INICIO_Y;
		escadas_y[3] = 103;//+INICIO_Y;
	
		Eddie_x = 0;
		Eddie_y = INICIO_Y+10;
		Eddie_last_x = 0;
		Eddie_last_x = 0;
	
	
		level = 0;
		
	
		jumping = false;
		jumping_counter = 0;
		jumping_side = 0;
		may_go_up = false;
		may_go_down = false;
		going_up = false;
		going_up = false;
		going_up_counter = 0;
		going_down_counter = 0;
	
	
		GrContextInit(&sContext, &g_sCfaf128x128x16);
		GrFlush(&sContext);
		GrContextFontSet(&sContext, g_psFontFixed6x8);
		
		InitializeBacKGround();
}

void EddieController(){
		uint16_t x, y;
		char pbufx[10], pbufy[10];
		bool jumping_button;
		
		Eddie_last_x = Eddie_x;
		Eddie_last_y = Eddie_y;
	
		if(going_up==false && going_down==false && jumping==false){
				//x = joy_read_x();
				//y = joy_read_y();
				
				x = 0.0048*joy_read_x();
				y = -0.0029*joy_read_y() + 12;
				jumping_button = button_read_debounce();
			
				if(jumping_button){	//jumping_side: 0=parado, 1=direita, 2=esquerda
						jumping=true;
						if(x>13){jumping_side=1;}
						else if(x<5){jumping_side=2;}
						else{jumping_side=0;}
				}
				if(jumping_button==false){
						if(x>13 && Eddie_x+EDDIE_PASSO<128-EDDIE_WIDTH){Eddie_x+=EDDIE_PASSO;}
						else if(x<5 && Eddie_x-EDDIE_PASSO>0){Eddie_x-=EDDIE_PASSO;}
						
						if(y>8 && may_go_down){
								//Eddie_y++;
								going_down = true;
						}
						else if(y<3 && may_go_up){
								//Eddie_y--;
								going_up = true;
						}
				}
				
				
		}
		
		else if(jumping){
				if(jumping_counter<6){
						if(jumping_counter==0 || jumping_counter==1 ){Eddie_y -=EDDIE_PASSO_JUMPING;}
						else if(jumping_counter==4 || jumping_counter==5 ){Eddie_y +=EDDIE_PASSO_JUMPING;}
						
						if(jumping_side==1 && Eddie_x+EDDIE_PASSO<128-EDDIE_WIDTH){Eddie_x += EDDIE_PASSO;}
						else if(jumping_side==  2 && Eddie_x-EDDIE_PASSO>0){Eddie_x -= EDDIE_PASSO;}
						
						jumping_counter++;
				}
				else{
						jumping_counter=0;
						jumping_side=0;
						jumping= false;
				}
		}
		
		
		
		else if(going_down){
				if(going_down_counter<5){
						Eddie_y +=EDDIE_PASSO;
						going_down_counter++;
				}
				else{
						Eddie_y +=EDDIE_PASSO-1;
						going_down_counter=0;
						going_down = false;
				}
		}
		else if(going_up){
				if(going_up_counter<5){
						Eddie_y -=EDDIE_PASSO;
						going_up_counter++;
				}
				else{
						Eddie_y -=EDDIE_PASSO-1;
						going_up_counter=0;
						going_up = false;
				}
		}
		
		if(Eddie_last_x!=Eddie_x || Eddie_last_y!=Eddie_y){
			drawBack(Eddie_last_x, Eddie_last_y, EDDIE_WIDTH, EDDIE_HEIGHT);		
		}
					
		GrContextBackgroundSet(&sContext, ClrBlack);
		GrContextForegroundSet(&sContext, ClrAquamarine);
		draw_Eddie(Eddie_x, Eddie_y);
		
		intToString(Eddie_x, pbufx, 10, 10, 3);
		intToString(Eddie_y, pbufy, 10, 10, 3);
		GrContextForegroundSet(&sContext, ClrRed);
		GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*1,  (sContext.psFont->ui8Height+2)*0, true);
		GrStringDraw(&sContext,(char*)pbufy, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*0, true);
		
}

void collisionStairsController(){
		uint8_t i;
		may_go_up=false;
		for(i=0; i<ESCADA_COUNTER ; i++){
				if(Eddie_x+EDDIE_WIDTH>escadas_x[i] && Eddie_x<escadas_x[i]+ESCADA_WIDTH && Eddie_y+EDDIE_HEIGHT>escadas_y[i] && Eddie_y<escadas_y[i]+ESCADA_HEIGHT){
						may_go_up = true;
				}
		}
		may_go_down=false;
		for(i=0; i<ESCADA_COUNTER ; i++){
				if(Eddie_x+EDDIE_WIDTH>escadas_x[i] && Eddie_x<escadas_x[i]+ESCADA_WIDTH && Eddie_y+EDDIE_HEIGHT+4>escadas_y[i] && Eddie_y<escadas_y[i]){
						may_go_down = true;
				}
		}
		
}
void GameController(){
	
		while(1){
				EddieController();
				collisionStairsController();
				if(may_go_up){
						GrContextForegroundSet(&sContext, ClrBlueViolet);
						draw_circle(0, 1);
						fill_circle(0, 1);
				}
				else if(may_go_down){
						GrContextForegroundSet(&sContext, ClrCoral);
						draw_circle(0, 1);
						fill_circle(0, 1);
				}
				else{
						GrContextForegroundSet(&sContext, ClrBlack);
						draw_circle(10, 1);
						fill_circle(10, 1);
				}
				if(jumping){
						GrContextForegroundSet(&sContext, ClrAzure);
						draw_circle(5, 1);
						fill_circle(5, 1);
				}
				else{
						GrContextForegroundSet(&sContext, ClrBlack);
						draw_circle(5, 1);
						fill_circle(5, 1);
				}
				
			osDelay(500);
		}	


}

int main (void) {

  uint32_t color = 0x00, i, j, pixel,pixel_ed;
	
	init_all();
	Initialize();
	GameController();
	
	
	
}
