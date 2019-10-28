/*============================================================================
 *                    Exemplos de utilização do Kit
 *              EK-TM4C1294XL + Educational BooterPack MKII 
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
#include "cmsis_os.h"

#include "math.h"

#define ZERO 78
#define MAX_V 3.3
#define MAX_F 200
#define DISPLAY_TAM 128
#define MAX_SIGNAL_HEIGHT 50
#define L1 32
#define L2 64
#define L3 96
#define SCALE_DEFAULT 4

#define F_PWM 2560

#define TEMPOTELA 0.05

tContext sContext;


typedef struct{
	float amplitude;
	uint16_t  waveType;
	uint16_t fn;
}Signal;

osPoolId sig_id;
osPoolDef(sig,10,Signal);

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
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
}


void draw_pixel(uint16_t x, uint16_t y){
	GrPixelDraw(&sContext, x, y);
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


/*-----------------new code*/
bool button_read_debounce1(void) {
	uint8_t i = 0;
	while(i < 50) {
		if(button_read_s1())
			i++;
		else
			return false;
	}
	return true;
}
bool button_read_debounce2(void) {
	uint8_t i = 0;
	while(i < 50) {
		if(button_read_s2())
			i++;
		else
			return false;
	}
	return true;
}




void clearDisplay(uint16_t amplitude, uint16_t scale){
		uint16_t i,j;
		
		for(i=0; i<DISPLAY_TAM; i++){
				GrContextForegroundSet(&sContext, ClrBlack);
				for(j=ZERO-amplitude; j<ZERO+amplitude+2; j++){
						draw_pixel(i,j);
				}
				GrContextForegroundSet(&sContext, ClrRed);
				draw_pixel(i,ZERO);//linha horizontal
		}	
		for(i=0; i<DISPLAY_TAM; i++){
				if(((L1*scale/SCALE_DEFAULT)>0) && ((L1*scale/SCALE_DEFAULT)<128)){
						draw_pixel((L1*scale/SCALE_DEFAULT),j);
				}
				if(((L2*scale/SCALE_DEFAULT)>0) && ((L2*scale/SCALE_DEFAULT)<128)){
						draw_pixel((L2*scale/SCALE_DEFAULT),j);
				}
				if(((L3*scale/SCALE_DEFAULT)>0) && ((L3*scale/SCALE_DEFAULT)<128)){
						draw_pixel((L3*scale/SCALE_DEFAULT),j);
				}
		}
}




char PrintCaracteristics(uint16_t wt, uint16_t f, float amp){
		char pbuf_f[10], pbuf_a[10];
		char wave[5];
	
		//WaveType: 0->senoide; 1->triangular; 2->dente de serra; 3->quadrada; 4->trapezoidal
		GrStringDraw(&sContext,"WaveType:", -1, (sContext.psFont->ui8MaxWidth)*0,  (sContext.psFont->ui8Height+2)*0, true);
		if(wt==0){
				GrStringDraw(&sContext,"Sine", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
		}
		if(wt==1){
				GrStringDraw(&sContext,"Triangle", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
		}
		if(wt==2){
				GrStringDraw(&sContext,"SawTooth", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
		}
		if(wt==3){
				GrStringDraw(&sContext,"Square", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
		}
		if(wt==4){
				GrStringDraw(&sContext,"Trapezoid", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*0, true);
		}
		
		//frequency
		intToString(f, pbuf_f, 10, 10, 3);
		GrStringDraw(&sContext,"Freq:", -1, (sContext.psFont->ui8MaxWidth)*0,  (sContext.psFont->ui8Height+2)*1, true);
		GrStringDraw(&sContext,pbuf_f, -1, (sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*1, true);
		//amp
		floatToString(amp, pbuf_a, 10, 10, 3, 3);
		GrStringDraw(&sContext,"Ampl:", -1, (sContext.psFont->ui8MaxWidth)*10,  (sContext.psFont->ui8Height+2)*1, true);
		GrStringDraw(&sContext,pbuf_a, -1, (sContext.psFont->ui8MaxWidth)*16,  (sContext.psFont->ui8Height+2)*1, true);
} 

void thread_DrawDisplay(uint16_t x, float y){
		uint16_t i,j;
		uint16_t altura;
		
		if(y<ZERO){
				altura = 128-(int)y;
				for(j=ZERO; j<altura+1;j++)
						draw_pixel(i,j);		
		}
		else if(y>ZERO){
				altura = 128-(int)y;
				for(j=altura; j<ZERO+1;j++)
						draw_pixel(i,j);							
		}
		else{
				draw_pixel(i,ZERO);
		}	
}	

void thread_GenerateSignal(Signal *signal, uint16_t h_scale, float old_amp){
		
			uint16_t fs;
			float amp;
			float y[128], output[128];
			uint16_t i,j, altura;
			uint16_t tam;
			double inc_t;
			
			amp = (signal->amplitude)*(MAX_SIGNAL_HEIGHT/MAX_V);
			tam = (int) F_PWM/signal->fn;
			h_scale=4;
			clearDisplay((uint16_t)old_amp, h_scale);
			inc_t = 0.0078125;
	
			for(i=0; i<tam; i++){
				//WaveType: 0->senoide; 1->triangular; 2->dente de serra; 3->quadrada; 4->trapezoidal
					if(signal->fn==0)
						y[i]=1;
				
					//WaveType=senoide
					else if(signal->waveType==0){
							y[i]=sin(i*(2*PI/tam));
					}
					//WaveType=triangular
					else if(signal->waveType==1){
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
					else if(signal->waveType==2){
							y[i]= i*(inc_t);
					}
					//WaveType=Quadrada
					else if(signal->waveType==3){
							if(i<tam/2)
									y[i] = 1;
							else
									y[i] = -1;
					}		
					//WaveType=Trapezoidal
					else if(signal->waveType==4){
							if(i<(tam/4)){
									y[i] = i*(4*inc_t);
							}
							else if(i==(tam/4)){
									y[i]=1;
							}
							else if((i>(tam/4)) && (i<=(tam/4)*3)){
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
		
			GrContextForegroundSet(&sContext, ClrWhite);
			
			//regiao critica, usar mutex
			PrintCaracteristics(signal->waveType, signal->fn, signal->amplitude);
			//fim da região critica
			
			for(i=0; i<128; i++){
					output[i] = ZERO+amp*y[(i*h_scale/SCALE_DEFAULT)%(tam)];
					thread_DrawDisplay(i, output[i]);
				/*
					if(output[i]<ZERO){
							altura = 128-(int)output[i];
							for(j=ZERO; j<altura+1;j++)
									draw_pixel(i,j);		
					}
					else if(output[i]>ZERO){
							altura = 128-(int)output[i];
							for(j=altura; j<ZERO+1;j++)
									draw_pixel(i,j);							
					}
					else{
							draw_pixel(i,ZERO);
					}	*/
					osDelay(1000/tam);
			}
}

void thread_NewGenerateSignal(Signal *signal, uint16_t h_scale, float old_amp){
		
			uint16_t fs;
			float amp;
			float y[128], output[128];
			uint16_t i,j, altura;
			uint16_t tam;
			double inc_t;
			
	
			amp = (signal->amplitude)*(MAX_SIGNAL_HEIGHT/MAX_V);
			tam = (int) F_PWM/signal->fn;
			h_scale=4;
			clearDisplay((uint16_t)old_amp, h_scale);
			inc_t = 0.0078125;
	
			for(i=0; i<tam; i++){
				//WaveType: 0->senoide; 1->triangular; 2->dente de serra; 3->quadrada; 4->trapezoidal
					if(signal->fn==0)
						y[i]=1;
				
					//WaveType=senoide
					else if(signal->waveType==0){
							y[i]=sin(i*(2*PI/tam));
					}
					//WaveType=triangular
					else if(signal->waveType==1){
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
					else if(signal->waveType==2){
							y[i]= i*(inc_t);
					}
					//WaveType=Quadrada
					else if(signal->waveType==3){
							if(i<tam/2)
									y[i] = 1;
							else
									y[i] = -1;
					}		
					//WaveType=Trapezoidal
					else if(signal->waveType==4){
							if(i<(tam/4)){
									y[i] = i*(4*inc_t);
							}
							else if(i==(tam/4)){
									y[i]=1;
							}
							else if((i>(tam/4)) && (i<=(tam/4)*3)){
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
		
			GrContextForegroundSet(&sContext, ClrWhite);
			
			//regiao critica, usar mutex
			PrintCaracteristics(signal->waveType, signal->fn, signal->amplitude);
			//fim da região critica
			
			for(i=0; i<128; i++){
					output[i] = ZERO+amp*y[(i*h_scale/SCALE_DEFAULT)%(tam)];
					//thread_DrawDisplay(i, output[i]);
					
					if(output[i]<ZERO){
							altura = 128-(int)output[i];
							for(j=ZERO; j<altura+1;j++)
									draw_pixel(i,j);		
					}
					else if(output[i]>ZERO){
							altura = 128-(int)output[i];
							for(j=altura; j<ZERO+1;j++)
									draw_pixel(i,j);							
					}
					else{
							draw_pixel(i,ZERO);
					}
					osDelay(1/(signal->fn*128));
			}
}

int main (void) {
		uint16_t h_scale;
		float old_a;
		Signal *signal;
		bool hasChanges=false;
		sig_id = osPoolCreate(osPool(sig));
		
		init_all();
		
		h_scale=4;
			
		signal = (Signal*) osPoolAlloc(sig_id);
		signal->amplitude=3.3;
		signal->fn=20;
		signal->waveType=1;
			
		old_a = (signal->amplitude)*(MAX_SIGNAL_HEIGHT/MAX_V);
		
		thread_GenerateSignal(signal,h_scale, old_a);
		
		while(true){
			
				if(button_read_debounce1()){
						//h_scale*=2;				//para mudar a escala horizontal
						old_a = (signal->amplitude)*(MAX_SIGNAL_HEIGHT/MAX_V);	
						//signal->amplitude+=0.5;		//para mudar a amplitude do sinal
						
						signal->waveType+=1;
						if(signal->waveType==4){
								signal->waveType=0;
						}
					
						hasChanges=true;
				}
				if(button_read_debounce2()&& signal->amplitude>0){
						//h_scale/=2;
						old_a = (signal->amplitude)*(MAX_SIGNAL_HEIGHT/MAX_V);
						//signal->amplitude-=0.5;
						hasChanges=true;
				}
				
				if(hasChanges==true){
						thread_GenerateSignal(signal,h_scale, old_a);
						hasChanges=false;	
				}
		}
}




