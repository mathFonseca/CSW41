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
#include "Background2.h"
#include "inimigo.h"
#include "point.h"
#include "boss.h"
#include "key.h"

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

#define ENEMY_WIDTH 5
#define ENEMY_HEIGHT 5
#define ENEMIES_COUNTER 4

#define BOSS_HEIGHT 9
#define BOSS_WIDTH 5

#define POINT_WIDTH 5
#define POINT_HEIGHT 5
#define POINT_COUNTER 2

#define KEY_WIDTH 5
#define KEY_HEIGHT 4

#define INICIO_Y 30
#define MARGEM_ESQUERDA 3 
#define MAXLEVEL 2

#define TEMPO_TIMER 500 // em milisegundos.
//#define GRANTT

//To print on the screen
tContext sContext;

//escadas
uint32_t escadas_x[ESCADA_COUNTER];
uint32_t escadas_y[ESCADA_COUNTER];
//Eddie
uint32_t Eddie_x;
uint32_t Eddie_y;
bool going_up;
uint8_t going_up_counter;
bool going_down;
uint8_t going_down_counter;
bool jumping;
uint8_t jumping_counter;
uint8_t jumping_side; //0=parado, 1=direita, 2=esquerda
//enemies
uint32_t enemies_x[ENEMIES_COUNTER];
uint32_t enemies_y[ENEMIES_COUNTER];
uint8_t enemies_side[ENEMIES_COUNTER];//0=parado, 1=direita, 2=esquerda
//Boss
uint32_t Boss_x;
uint32_t Boss_y;
uint32_t Key_y;
uint8_t Boss_side;

//POINTS
uint32_t points_x[POINT_COUNTER];
uint32_t points_y[POINT_COUNTER];
bool points_move[POINT_COUNTER];

//game controller
uint8_t level;
uint8_t points;
uint8_t boss_height;
uint8_t lives;
uint16_t score;
bool game_over;
bool game_won;
bool onPause;


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

void init_all()
{
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


void draw_pixel(uint16_t x, uint16_t y)
{
	GrPixelDraw(&sContext, x, y);
}

void draw_Eddie(uint16_t x, uint16_t y){
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

void draw_Enemy(uint16_t x, uint16_t y){
	uint32_t i, j;
	uint32_t color, pixel=0;

	for(j=y; j<y+ENEMY_HEIGHT; j++){ //y
			for(i=x; i<x+ENEMY_WIDTH; i++){//x
					pixel = inimigo_start+3*((j-y)*ENEMY_WIDTH+i-x);
					color = 0x00;
					color += inimigo[pixel+2];
					color += inimigo[pixel+1]*16*16;
					color += inimigo[pixel]*16*16*16*16;
					color += 0x00*16*16*16*16*16*16;
					GrContextForegroundSet(&sContext, color);
					if(color!=ClrBlack)
						draw_pixel(i, j);
			}
	}
}

void draw_Boss(uint16_t x, uint16_t y){
	uint32_t i, j;
	uint32_t color, pixel=0;
	uint8_t extra = points;
	if(extra>4)
			extra = 4;
	
	for(j=y; j<y+BOSS_HEIGHT-extra; j++){ //y
			for(i=x; i<x+BOSS_WIDTH; i++){//x
					pixel = boss_start+3*((j-y)*BOSS_WIDTH+i-x);
					color = 0x00;
					color += boss[pixel+2];
					color += boss[pixel+1]*16*16;
					color += boss[pixel]*16*16*16*16;
					color += 0x00*16*16*16*16*16*16;
					GrContextForegroundSet(&sContext, color);
					if(color!=ClrBlack)
						draw_pixel(i, j);
			}
	}
}

void draw_Point(uint16_t x, uint16_t y){
	uint32_t i, j;
	uint32_t color, pixel=0;

	for(j=y; j<y+POINT_HEIGHT; j++){ //y
			for(i=x; i<x+POINT_WIDTH; i++){//x
					pixel = point_start+3*((j-y)*POINT_WIDTH+i-x);
					color = 0x00;
					color += point[pixel+2];
					color += point[pixel+1]*16*16;
					color += point[pixel]*16*16*16*16;
					color += 0x00*16*16*16*16*16*16;
					GrContextForegroundSet(&sContext, color);
					if(color!=ClrBlack)
						draw_pixel(i, j);
			}
	}
}
void draw_Key(uint16_t x, uint16_t y){
	uint32_t i, j;
	uint32_t color, pixel=0;

	for(j=y; j<y+KEY_HEIGHT; j++){ //y
			for(i=x; i<x+KEY_WIDTH; i++){//x
					pixel = key_start+3*((j-y)*KEY_WIDTH+i-x);
					color = 0x00;
					color += key[pixel+2];
					color += key[pixel+1]*16*16;
					color += key[pixel]*16*16*16*16;
					color += 0x00*16*16*16*16*16*16;
					GrContextForegroundSet(&sContext, ClrYellow);
					if(color!=ClrBlack)
						draw_pixel(i, j);
			}
	}
}


void drawBack(uint16_t x, uint16_t y,uint16_t width, uint16_t height){
	uint32_t i, j;
	uint32_t color, pixel;
	
	for(j=y; j<y+height; j++){ //y
		for(i=x; i<x+width; i++){//x
				if(level==1){
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
				else if(level==2){
						pixel = Background2_start+3*((j)*128+i);
						color = 0x00;
						color += Background2[pixel+2];
						color += Background2[pixel+1]*16*16;
						color += Background2[pixel]*16*16*16*16;
						color += 0x00*16*16*16*16*16*16;
						GrContextForegroundSet(&sContext, color);	
						draw_pixel(i, j);
				}
				
		}
	}
}
void draw(uint8_t id, uint16_t x, uint16_t y, uint16_t last_x, uint16_t last_y, uint8_t key_y, uint8_t last_key_y){
		
		/*id: 0 - drawBack
					1 - draw_Eddie
					2 - draw_Enemy
					3 - draw_Point 
					4 - draw_Boss && draw_Key	*/
		
		if(id==0){// caso precise no futuro
		}
		else if (id==1){
				if(last_x!=x || last_y!=y)
					drawBack(last_x, last_y, EDDIE_WIDTH, EDDIE_HEIGHT);		
				draw_Eddie(x, y);		
		}
		else if (id==2){
				if(last_x!=x || last_y!=y)
					drawBack(last_x, last_y, ENEMY_WIDTH, ENEMY_HEIGHT);		
				draw_Enemy(x, y);		
		}
		else if (id==3){
				if(last_x!=x || last_y!=y)
					drawBack(last_x, last_y, POINT_WIDTH, POINT_HEIGHT);		
				draw_Point(x, y);		
		}
		else if (id==4){
				if(last_x != x || last_y != y){
						drawBack(last_x, last_y, BOSS_WIDTH, BOSS_HEIGHT);
						drawBack(last_x, last_key_y, KEY_WIDTH, KEY_HEIGHT);
				}
				draw_Boss(x,y);			
				draw_Key(x,Key_y);
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
void InitializeBackGround(){
		uint32_t color = 0x00, i, j, pixel;
	
		if(level==1){
				pixel = Background_start;
				for(j = INICIO_Y; j < 128; j++){ //y
					for(i = 0; i < 128; i++){ //x
						//pixel = Background_start+3*((j-INICIO_Y)*128+i);
						color = 0x00;
						color += Background[pixel+2];
						color += Background[pixel+1]*16*16;
						color += Background[pixel]*16*16*16*16;
						color += 0x00*16*16*16*16*16*16;
						#ifdef GRANTT
						GrContextForegroundSet(&sContext, color);
						draw_pixel(i, j);
						#endif
						pixel += 3;
					}
				}
		}
		else if(level==2){
				pixel = Background2_start;
				for(j = 0; j < 128; j++){ //y
						for(i = 0; i < 128; i++){ //x
							color = 0x00;
							color += Background2[pixel+2];
							color += Background2[pixel+1]*16*16;
							color += Background2[pixel]*16*16*16*16;
							color += 0x00*16*16*16*16*16*16;
							#ifdef GRANTT
							GrContextForegroundSet(&sContext, color);
							draw_pixel(i, j);
							#endif
							pixel += 3;
						}
				}
		}
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
				#ifdef GRANTT
				GrContextForegroundSet(&sContext, color);
				draw_pixel(i, j);
				#endif
				pixel += 3;
			}
		}

}

void Initialize(){

		escadas_x[0] = 100;
		escadas_x[1] = 5;
		escadas_x[2] = 80;
		escadas_x[3] = 10;
	
		escadas_y[0] = 35;
		escadas_y[1] = 58;
		escadas_y[2] = 80;
		escadas_y[3] = 103;
		
	//enemies_side: 0=parado, 1=direita, 2=esquerda
		enemies_x[0] = 20;
		enemies_y[0] = 55-ENEMY_HEIGHT;
		enemies_side[0] = 2;
		enemies_x[1] = 40;
		enemies_y[1] = 55-ENEMY_HEIGHT+23;
		enemies_side[1] = 1;
		enemies_x[2] = 60;
		enemies_y[2] = 55-ENEMY_HEIGHT+46;
		enemies_side[2] = 0;
		enemies_x[3] = 10;
		enemies_y[3] = 55-ENEMY_HEIGHT+69;
		enemies_side[3] = 2;
		
		//POINTS
		points_x[0] = 35;
		points_y[0] = 35;
		points_move[0] = false;
		points_x[1] = 35;
		points_y[1] = 35+46;
		points_move[0] = true;
	
		Eddie_x = 115;
		Eddie_y = 124-EDDIE_HEIGHT;
		
		Boss_x = (128-MARGEM_ESQUERDA)/2+MARGEM_ESQUERDA;
		Boss_y = 33-BOSS_HEIGHT;
		Key_y = Boss_y - KEY_HEIGHT;
		Boss_side = 1;
		
		level = 1;
		points = 0;
		boss_height = 10;
		lives = 5;
		score=0;
		game_over = false;
		game_won = false;
		onPause = false;
		
		jumping = false;
		jumping_counter = 0;
		jumping_side = 0;
		going_up = false;
		going_up = false;
		going_up_counter = 0;
		going_down_counter = 0;
	
		#ifdef GRANTT
		GrContextInit(&sContext, &g_sCfaf128x128x16);
		GrFlush(&sContext);
		GrContextFontSet(&sContext, g_psFontFixed6x8);
		#endif
		
		
}
bool collisionStairsControllerUp(){
		uint8_t i;
		for(i=0; i<ESCADA_COUNTER ; i++){
				if(Eddie_x+EDDIE_WIDTH>escadas_x[i] && Eddie_x<escadas_x[i]+ESCADA_WIDTH && Eddie_y+EDDIE_HEIGHT>escadas_y[i] && Eddie_y<escadas_y[i]+ESCADA_HEIGHT){
						return true;
				}
		}
		return false;
}
bool collisionStairsControllerDown(){
		uint8_t i;
		for(i=0; i<ESCADA_COUNTER ; i++){
				if(Eddie_x+EDDIE_WIDTH>escadas_x[i] && Eddie_x<escadas_x[i]+ESCADA_WIDTH && Eddie_y+EDDIE_HEIGHT+4>escadas_y[i] && Eddie_y<escadas_y[i]){
						return true;
				}
		}	
		return false;
}
void EddieController(){
		uint16_t x, y;
		char pbufx[10], pbufy[10];
		bool jumping_button, may_go_up,may_go_down;
		uint32_t Eddie_last_x;
		uint32_t Eddie_last_y;
	
		if(!onPause){
				Eddie_last_x = Eddie_x;
				Eddie_last_y = Eddie_y;
				may_go_up = collisionStairsControllerUp();
				may_go_down = collisionStairsControllerDown();
				if(going_up==false && going_down==false && jumping==false){
						x = 0.0048*joy_read_x();
						y = -0.0029*joy_read_y() + 12;
						#ifdef GRANTT
						jumping_button = button_read_debounce();
						#endif
					
						if(jumping_button){	//jumping_side: 0=parado, 1=direita, 2=esquerda
								jumping=true;
								if(x>13){jumping_side=1;}
								else if(x<5){jumping_side=2;}
								else{jumping_side=0;}
						}
						if(jumping_button==false){
								if(x>13 && Eddie_x+EDDIE_PASSO<128-EDDIE_WIDTH){
										Eddie_x+=EDDIE_PASSO;
								}
								if(x<5 && Eddie_x-EDDIE_PASSO>MARGEM_ESQUERDA){
										Eddie_x-=EDDIE_PASSO;
								}
								if(y>8 && may_go_down){
										going_down = true;
								}
								if(y<3 && may_go_up){
										going_up = true;
								}
								
						}
				}
				
				else if(jumping){
						if(jumping_counter<8){
								if(jumping_counter==0 || jumping_counter==1 ){Eddie_y -=EDDIE_PASSO_JUMPING;}
								else if(jumping_counter==6 || jumping_counter==7 ){Eddie_y +=EDDIE_PASSO_JUMPING;}
								
								if(jumping_side==1 && Eddie_x+EDDIE_PASSO<128-EDDIE_WIDTH){Eddie_x += EDDIE_PASSO;}
								else if(jumping_side==  2 && Eddie_x-EDDIE_PASSO>MARGEM_ESQUERDA){Eddie_x -= EDDIE_PASSO;}
								
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
				
				//if(Eddie_last_x!=Eddie_x || Eddie_last_y!=Eddie_y){
				//	drawBack(Eddie_last_x, Eddie_last_y, EDDIE_WIDTH, EDDIE_HEIGHT);		
				//}
				//draw_Eddie(Eddie_x, Eddie_y);		
				#ifdef GRANTT
				draw(1,Eddie_x, Eddie_y, Eddie_last_x, Eddie_last_y, 0, 0);
				#endif
		}
}


void EnemiesController(){
		uint16_t x, y, i;
		
		uint32_t enemies_last_x[ENEMIES_COUNTER] ;
		uint32_t enemies_last_y[ENEMIES_COUNTER] ;
		if(!onPause){
				for(i=0; i<ENEMIES_COUNTER; i++){
						enemies_last_x[i] = enemies_x[i];
						enemies_last_y[i] = enemies_y[i];
					
						if(enemies_side[i]==1){
								if(enemies_x[i]+EDDIE_PASSO<128-ENEMY_WIDTH){enemies_x[i]+=EDDIE_PASSO;}
								else{enemies_side[i]=2;}			
						}
						else if(enemies_side[i]==2){
								if(enemies_x[i]-EDDIE_PASSO>MARGEM_ESQUERDA){enemies_x[i]-=EDDIE_PASSO;}						
								else{enemies_side[i]=1;}			
						}
						
						//if(enemies_last_x[i] != enemies_x[i] || enemies_last_y[i] != enemies_y[i]){
						//	drawBack(enemies_last_x[i], enemies_last_y[i], ENEMY_WIDTH, ENEMY_HEIGHT);		
						//}		
						//draw_Enemy(enemies_x[i],enemies_y[i]);
						#ifdef GRANTT
						draw(2, enemies_x[i],enemies_y[i],enemies_last_x[i], enemies_last_y[i], 0, 0);
						#endif
				}		
		}
}


void PointsController(){
		uint16_t  i;
		
		uint32_t points_last_x[POINT_COUNTER] ;
		uint32_t points_last_y[POINT_COUNTER] ;
		if(!onPause){
				for(i=0; i<POINT_COUNTER; i++){
						points_last_x[i] = points_x[i];
						points_last_y[i] = points_y[i];
					
						if(points_move[i]==true){
								if(points_x[i]+EDDIE_PASSO<128-POINT_WIDTH){points_x[i]+=EDDIE_PASSO;}
								else{	points_x[i] = 0;}			
						}
						
						//if(points_last_x[i] != points_x[i] || points_last_y[i] != points_y[i]){
						//	drawBack(points_last_x[i], points_last_y[i], POINT_WIDTH, POINT_HEIGHT);		
						//}		
						//draw_Point(points_x[i],points_y[i]);
						#ifdef GRANTT
						draw(3, points_x[i],points_y[i], points_last_x[i], points_last_y[i], 0, 0);
						#endif
				}
		}		
}


void BossController(){
		uint16_t x, y, i;
		uint8_t extra=0;
		uint32_t boss_last_x;
		uint32_t boss_last_y;
		uint32_t key_last_y;
	
		if(!onPause){
				boss_last_x = Boss_x;
				boss_last_y = Boss_y;
				key_last_y = Key_y;
			
				if(Boss_side==1){
						if(Boss_x+EDDIE_PASSO<128-BOSS_WIDTH){Boss_x+=EDDIE_PASSO;}
						else{Boss_side=2;}			
				}
				else if(Boss_side==2){
						if(Boss_x-EDDIE_PASSO>MARGEM_ESQUERDA){Boss_x-=EDDIE_PASSO;}						
						else{Boss_side=1;}			
				}
				
				extra = points;
				if(extra>4)
						extra = 4;
				Boss_y = 33-BOSS_HEIGHT+extra;
				Key_y = Boss_y-KEY_HEIGHT;
				
				//if(boss_last_x != Boss_x || boss_last_y != Boss_y){
				//	drawBack(boss_last_x, boss_last_y, BOSS_WIDTH, BOSS_HEIGHT);
				//	drawBack(boss_last_x, key_last_y, KEY_WIDTH, KEY_HEIGHT);
				//}
				//draw_Boss(Boss_x,Boss_y);			
				//draw_Key(Boss_x,Key_y);
				#ifdef GRANTT
				draw(4,Boss_x,Boss_y,boss_last_x, boss_last_y,Key_y,key_last_y );
				#endif
		}
}
		

uint8_t collisionEnemyController(){
		uint8_t i;
		for(i=0; i<ENEMIES_COUNTER; i++){
				if(Eddie_x+EDDIE_WIDTH>enemies_x[i] && Eddie_x<enemies_x[i]+ENEMY_WIDTH && Eddie_y+EDDIE_HEIGHT>enemies_y[i] && Eddie_y<enemies_y[i]+ENEMY_HEIGHT){
					return i;
				}
		}
		return 10; //codigo de erro
}

uint8_t collisionPointController(){
		uint8_t i;
		for(i=0; i<POINT_COUNTER; i++){
				if(Eddie_x+EDDIE_WIDTH>points_x[i] && Eddie_x<points_x[i]+POINT_WIDTH && Eddie_y+EDDIE_HEIGHT>points_y[i] && Eddie_y<points_y[i]+POINT_HEIGHT){
						return i;
				}
		}	
		return 10; //codigo de erro
}

bool collisionKeyController(){
		uint8_t i;
		if(Eddie_x+EDDIE_WIDTH>Boss_x && Eddie_x<Boss_x+KEY_WIDTH && Eddie_y+EDDIE_HEIGHT>Key_y && Eddie_y<Key_y+KEY_HEIGHT){
				return true;
		}
		return false;
}
bool collisionBossController(){
		uint8_t i;
		uint8_t boss_h;
		uint8_t extra = points;
		if(extra>4)
				extra = 4;
		boss_h = BOSS_HEIGHT-extra;
		if(Eddie_x+EDDIE_WIDTH>Boss_x && Eddie_x<Boss_x+BOSS_WIDTH && Eddie_y+EDDIE_HEIGHT>Boss_y && Eddie_y<Boss_y+boss_h){
				return true;
		}
		return false;
}


void ResetEddie(){
		//drawBack(Eddie_x, Eddie_y, EDDIE_WIDTH,EDDIE_HEIGHT);
		uint16_t last_x,last_y;
		last_x = Eddie_x;
		last_y = Eddie_y;
		Eddie_x = 115;
		Eddie_y = 124-EDDIE_HEIGHT;
		#ifdef GRANTT
		draw(1,Eddie_x, Eddie_y,last_x,last_y,0,0);
		#endif
		
		jumping = false;
		jumping_counter = 0;
		jumping_side = 0;
		going_up = false;
		going_up = false;
		going_up_counter = 0;
		going_down_counter = 0;
}
void newPointPosition(uint8_t id){
		uint8_t new_x, new_y;
		//drawBack(points_x[id], points_y[id], ENEMY_WIDTH, ENEMY_HEIGHT);
		new_x = (score*points_x[id]/13)%120+MARGEM_ESQUERDA; 
		new_y = points_y[id]+23;
		if(new_y > 110){
				new_y = 35;
		}
		#ifdef GRANTT
		draw(3,new_x,new_y, points_x[id], points_y[id], 0,0 );
		#endif
		points_x[id] = new_x;
		points_y[id] = new_y;
	
}
void newEnemyPosition(uint8_t id){
		//drawBack(enemies_x[id], enemies_x[id], ENEMY_WIDTH, ENEMY_HEIGHT);
		uint16_t last_x = enemies_x[id];
		if(enemies_side[id]!=0)
		{
			
				uint8_t new_x = (score*enemies_x[id]/(id+1))%120+MARGEM_ESQUERDA; 
				enemies_x[id] = new_x;
				
		}
		#ifdef GRANTT
		draw(2,enemies_x[id],enemies_y[id], last_x,points_y[id], 0,0 );
		#endif
		
}
void InitializeNewLevel()
{
		uint8_t i;
		if(level==2){
				escadas_x[0] = 33;
				escadas_x[1] = 95;
				escadas_x[2] = 3;
				escadas_x[3] = 49;
			
				escadas_y[0] = 35;
				escadas_y[1] = 58;
				escadas_y[2] = 80;
				escadas_y[3] = 103;
				
				//enemies_side: 0=parado, 1=direita, 2=esquerda
				for(i=0; i<ENEMIES_COUNTER; i++){
						newEnemyPosition(i);
				}
				for(i=0; i<POINT_COUNTER; i++){
						newPointPosition(i);
				}
				
				Eddie_x = 115;
				Eddie_y = 124-EDDIE_HEIGHT;
				
				Boss_x = (128-MARGEM_ESQUERDA)/2+MARGEM_ESQUERDA;
				Boss_y = 33-BOSS_HEIGHT;
				Key_y = Boss_y - KEY_HEIGHT;
				Boss_side = 1;
				
				jumping = false;
				jumping_counter = 0;
				jumping_side = 0;
				going_up = false;
				going_up = false;
				going_up_counter = 0;
				going_down_counter = 0;
		}	
		
		InitializeBackGround();
}
void OnPause()
{
		bool pause=true;
		#ifdef GRANTT
		GrContextForegroundSet(&sContext, ClrWhite);
		#endif
		while(pause)
		{
				#ifdef GRANTT
				pause = !button_read_debounce();
				GrStringDraw(&sContext,"Jump to start!", -1, (sContext.psFont->ui8MaxWidth)*4,  (sContext.psFont->ui8Height+2)*6, true);
				#endif
		}

}
void GameOver(){
		char buf[10];
		uint16_t i,j;
		bool pause=true;
		#ifdef GRANTT
		GrContextForegroundSet(&sContext, ClrBlack);
		#endif
		intToString(score, buf, 10, 10, 3);
		
		for (i=0; i<128; i++)
		{
				for (j=0; j<128; j++)
				{
						#ifdef GRANTT
							draw_pixel(i, j);
						#endif
				}
		}
		GrContextForegroundSet(&sContext, ClrYellow);
		while(pause)
		{
				#ifdef GRANTT
				pause = !button_read_debounce();
				GrStringDraw(&sContext,"Game Over", -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*4, true);
				GrStringDraw(&sContext,"U LOSER", -1, (sContext.psFont->ui8MaxWidth)*7,  (sContext.psFont->ui8Height+2)*5, true);
				GrStringDraw(&sContext,"Maybe next time?", -1, (sContext.psFont->ui8MaxWidth)*2,  (sContext.psFont->ui8Height+2)*6, true);
				GrStringDraw(&sContext,"Score:", -1, (sContext.psFont->ui8MaxWidth)*5,  (sContext.psFont->ui8Height+2)*7, true);
				GrStringDraw(&sContext,buf, -1, (sContext.psFont->ui8MaxWidth)*13,  (sContext.psFont->ui8Height+2)*7, true);
				#endif
		}
}
void GameWon()
{
		char buf[10];
		uint16_t i,j;
		bool pause=true;
		#ifdef GRANTT
			GrContextForegroundSet(&sContext, ClrBlack);
		#endif
		
		intToString(score, buf, 10, 10, 3);
		
		for (i=0; i<128; i++)
		{
				for (j=0; j<128; j++)
				{
					#ifdef GRANTT
						draw_pixel(i, j);
					#endif
				}
		}
		GrContextForegroundSet(&sContext, ClrYellow);
		while(pause)
		{
				#ifdef GRANTT
				pause = !button_read_debounce();
				GrStringDraw(&sContext,"YOU WON!", -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*5, true);
				GrStringDraw(&sContext,"Congrats", -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*6, true);
				GrStringDraw(&sContext,"Your Score:", -1, (sContext.psFont->ui8MaxWidth)*2,  (sContext.psFont->ui8Height+2)*7, true);
				GrStringDraw(&sContext,buf, -1, (sContext.psFont->ui8MaxWidth)*15,  (sContext.psFont->ui8Height+2)*7, true);
				#endif
		}
}
void NextLevel()
{
		uint16_t i,j;
		char buf[10];
		bool pause=true;
		onPause = true;
		
		#ifdef GRANTT
			GrContextForegroundSet(&sContext, ClrBlack);
		#endif
		for (i=0; i<128; i++)
		{
				for (j=0; j<128; j++)
				{
						#ifdef GRANTT
							draw_pixel(i, j);
						#endif
				}
		}
		#ifdef GRANTT
			GrContextForegroundSet(&sContext, ClrWhite);
		#endif
		
		level++;
		points=0;
		intToString(level, buf, 10, 10, 3);
		while(pause)
		{
				#ifdef GRANTT
				pause = !button_read_debounce();
				GrStringDraw(&sContext,"Next Level!", -1, (sContext.psFont->ui8MaxWidth)*5,  (sContext.psFont->ui8Height+2)*4, true);
				GrStringDraw(&sContext,buf, -1, (sContext.psFont->ui8MaxWidth)*9,  (sContext.psFont->ui8Height+2)*5, true);
				GrStringDraw(&sContext,"Jump to Continue!", -1, (sContext.psFont->ui8MaxWidth)*3,  (sContext.psFont->ui8Height+2)*6, true);
				#endif
		}
		InitializeNewLevel();
		onPause = false;
}

void GameController()
{
		char pbufx[10], pbufy[10], pbufz[10];
		uint8_t id_enemy=10, id_point=10;
		bool boss_collision, key_collision;
		bool needPause = false;
		
		while(!game_over && !game_won)
		{
			//vira thread
				EddieController();
				EnemiesController();
				PointsController();
				BossController();
			
				//fica aqui msm 
				boss_collision = collisionBossController();
				key_collision = collisionKeyController();
				id_enemy =collisionEnemyController();
				id_point = collisionPointController();
				
				if(boss_collision || id_enemy!=10)
				{	//boss_collision || id_enemy!=10
						if(lives>1)
						{
								lives--;
								ResetEddie();
								newEnemyPosition(id_enemy);
								needPause = true;
						}
						else{game_over = true;}
					
				}
				if(id_point!=10)
				{
						points++;
						score+=10;
						newPointPosition(id_point);
				}
				if(key_collision )
				{
						if(level<MAXLEVEL)
						{
								points++;
								score+=300;
								NextLevel();
						}
						else
						{
								game_won = true;
						}	
				}
				#ifdef GRANTT
					GrContextBackgroundSet(&sContext, ClrBlack);
					intToString(lives, pbufx, 10, 10, 3);
					intToString(score, pbufy, 10, 10, 3);
					GrContextForegroundSet(&sContext, ClrRed);
					GrStringDraw(&sContext,"Lives:", -1, (sContext.psFont->ui8MaxWidth)*0,  (sContext.psFont->ui8Height+2)*0, true);
					GrStringDraw(&sContext,"Score", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
					GrStringDraw(&sContext,(char*)pbufx, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*0, true);
					GrStringDraw(&sContext,(char*)pbufy, -1, (sContext.psFont->ui8MaxWidth)*16,  (sContext.psFont->ui8Height+2)*0, true);
				#endif
			if(needPause)
			{
					osDelay(100);
					needPause = false;
			}	
			if(points>=4)
			{
					#ifdef GRANTT
						GrStringDraw(&sContext,"Get The Key, idiot!", -1, (sContext.psFont->ui8MaxWidth)*0,  (sContext.psFont->ui8Height+2)*1, true);
					#endif
			}
			osDelay(500);
		}	
		onPause = true;
		if(game_won){
				GameWon();
		}
		else{
			GameOver();
		}

}



/*
int main (void) {

  uint32_t color = 0x00, i, j, pixel,pixel_ed;
	
	init_all();
	Initialize();
	OnPause();
	InitializeBackGround();
	GameController();
}
*/

void t_GameController()
{
	while(1)
	{
		// todo conte?do aqui dentro.
		#ifdef GRANTT
			// Todo acesso ao I/O aqui dentro.
		#endif
	}
}
void t_EddieController()
{
	while(1)
	{
		// todo conte?do aqui dentro.
		#ifdef GRANTT
			// Todo acesso ao I/O aqui dentro.
		#endif
	}
}
void t_EnemiesController()
{
	while(1)
	{
		// todo conte?do aqui dentro.
		#ifdef GRANTT
			// Todo acesso ao I/O aqui dentro.
		#endif
	}
}
void t_PointsController()
{
	while(1)
	{
		// todo conte?do aqui dentro.
		#ifdef GRANTT
			// Todo acesso ao I/O aqui dentro.
		#endif
	}
}
void t_BossController()
{
	while(1)
	{
		// todo conte?do aqui dentro.
		#ifdef GRANTT
			// Todo acesso ao I/O aqui dentro.
		#endif
	}
}

void t_TimerCallBack()
{
		EddieController();
		EnemiesController();
		PointsController();
		BossController();
}

osThreadDef(t_GameController, osPriorityNormal, 1, 0);
//osThreadDef(t_EddieController, osPriorityNormal, 1, 0);
//osThreadDef(t_EnemiesController, osPriorityNormal, 1, 0);
//osThreadDef(t_PointsController, osPriorityNormal, 1, 0);
//osThreadDef(t_BossController, osPriorityNormal, 1, 0);

osTimerDef(clock, t_TimerCallBack);
osTimerId	clock_sistema;

osMutexId mutex_tela;
osMutexDef (mutex_tela);

int main (void)
{
	osKernelInitialize();
	#ifdef GRANTT
		init_all();
	#endif
	
	// Cria todas as threads
	osThreadCreate(osThread(t_GameController), NULL);
	//osThreadCreate(osThread(t_EddieController), NULL);
	//osThreadCreate(osThread(t_EnemiesController), NULL);
	//osThreadCreate(osThread(t_PointsController), NULL);
	//osThreadCreate(osThread(t_BossController), NULL);
	
	// Cria o timer do sistema
	clock_sistema = osTimerCreate(osTimer(clock), osTimerPeriodic, NULL);
	osTimerStart(clock_sistema, TEMPO_TIMER);
	
	// Cria os sem?foros do sistema
	
	// Cria os Mutex do sistema
	mutex_tela = osMutexCreate(osMutex(mutex_tela));
	
	osKernelStart();
	//osDelay(osWaitForever);
}



