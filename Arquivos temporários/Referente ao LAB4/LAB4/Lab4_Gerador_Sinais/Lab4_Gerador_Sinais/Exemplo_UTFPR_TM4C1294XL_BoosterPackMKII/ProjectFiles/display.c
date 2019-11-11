#include "Display.h"
#include "buzzer.h"

tContext sContext;

char *freq_antiga;

uint8_t tensao_antiga_antiga, tensao_antiga,i=0;

void initScreen(void){	
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);

}
void drawFunction(uint8_t tensao_ini,uint8_t tensao_fim,uint8_t posicao_tempo){
	GrContextForegroundSet(&sContext, ClrPink);
	
	GrLineDrawH(&sContext,42,84,49);
	GrLineDrawH(&sContext,42,84,121);	
	
	GrContextForegroundSet(&sContext, ClrBlack);
	GrLineDrawV(&sContext,posicao_tempo,50,120);
	//GrLineDrawV(&sContext,i,50,120);
//	
//	i++;
//	if( i >= 128)
//		i=0;
	
	if(tensao_fim < 50 || tensao_fim > 120){
		tensao_fim = -5;
		}
	if(tensao_ini < 50 || tensao_ini > 120){
		tensao_ini = -5;
		}
		GrContextForegroundSet(&sContext, ClrWhiteSmoke);
		GrLineDrawV(&sContext,posicao_tempo,tensao_fim,tensao_ini);
		//GrPixelDraw(&sContext,posicao_tempo,tensao_fim);

}

void drawEixo(){
	// Y
	GrContextForegroundSet(&sContext, ClrPink);
	GrLineDrawV(&sContext,2,0,127);
	// X
	GrContextForegroundSet(&sContext, ClrPink);
	GrLineDrawH(&sContext,0,127,125);
}

void drawFreqAmp(char* freq, char* amp){
	GrContextForegroundSet(&sContext, ClrBlack);
	GrStringDraw(&sContext,"¦¦¦",-1,100,5,1);
	GrContextForegroundSet(&sContext, ClrPink);
	GrStringDraw(&sContext,"Freq(Hz):",-1,40,5,1);
	GrStringDraw(&sContext,freq,-1,100,5,1);
	freq_antiga = freq;
	GrStringDraw(&sContext,"Tensao(V):",-1,40,15,1);
	GrStringDraw(&sContext,amp,-1,100,15,1);
	
}

void som( void ){
	buzzer_vol_set(5000);
	buzzer_per_set(2000);
	buzzer_write(true);
	osDelay(570);
}