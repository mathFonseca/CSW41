#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include "grlib/grlib.h"
#include "cfaf128x128x16.h"
#include "buttons.h"
#include "joy.h"
#include "airplane.h"
#include "bus.h"

tContext sContext;
uint8_t matriz_resultado[128][128];
void SysTick_Wait1ms(uint32_t delay);
extern void zoom_ass_2(uint8_t scale, bool image_choose, bool zoom_choose);

void init_functions(){
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
		GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
}

void draw_pixel(uint8_t x, uint8_t y){
		GrPixelDraw(&sContext, x, y);
}

void limpa_tela (){
	uint8_t i, j;
	for(i = 0; i<128; i++){
		for(j = 0; j<128; j++){
			GrContextForegroundSet(&sContext, ClrBlack);
			draw_pixel(j, i);
		}
	}
}
void limpaMatriz (uint8_t cor){
	uint8_t i, j;
	for(i = 0; i<128; i++){
		for(j = 0; j<128; j++){
			if(cor==0)
			{
				matriz_resultado[j][i] = 1;
			}
			else
			{
				matriz_resultado[j][i] = 0;
			}
			
		}
	}
}

void MostraTela (uint8_t cor){
	uint8_t i, j;
	for(i = 0; i<128; i++){
		for(j = 0; j<128; j++){
			if(cor==0){
				if(matriz_resultado[j][i]==1){
					GrContextForegroundSet(&sContext, ClrBlack);
				}
				else{
					GrContextForegroundSet(&sContext, ClrWhite);
				}
				draw_pixel(j, i);
			}
			else{
				if(matriz_resultado[j][i]==1){
					GrContextForegroundSet(&sContext, ClrWhite);
				}
				else{
					GrContextForegroundSet(&sContext, ClrBlack);
				}
				draw_pixel(j, i);
			}
		}
	}
}


void ZoomAssembly(uint8_t scale, bool image_choose, bool zoom_choose){
	
	uint32_t i, j, k, l;	//	Para movimentação dentro dos FOR.
	uint32_t pixel = 0;	// Para movimentação dentro da matriz da imagem.
	uint8_t pinta =0;
	
	if(zoom_choose == false){ // zoom_in
		for(i = 0; i < 64*scale; i+=scale){ // Linha (x)
			for(j = 0; j < 96*scale ; j+=scale){ // Coluna (y)
				if(j<128 && i<128){
						if((image_choose==false && (airplane[pixel] >= 0xF0 && airplane[pixel + 1] >= 0xF0 && airplane[pixel + 2] >= 0xF0)) ||
							 (image_choose==true && (bus[pixel] >= 0xF0 && bus[pixel + 1] >= 0xF0 && bus[pixel + 2] >= 0xF0)) )
							pinta=0;
						else
							pinta=1;
						
						for(k=i; k<(scale + i); k++){
							for(l=j; l<(scale + j); l++){
								if( l < 128 && k < 128)
									matriz_resultado[l][k] = pinta;
							}
						}
				}
				pixel+=3;
			}
		}		
	}
	else{	// zoom_out
		for(i = 0; i < 64 ; i+=scale){ // Linha (x)
			for(j = 0; j < 96 ; j+=scale){ // Coluna (y)
				if(j<128 && i<128){
						if((image_choose==false && (airplane[pixel] >= 0xF0 && airplane[pixel + 1] >= 0xF0 && airplane[pixel + 2] >= 0xF0)) ||
								 (image_choose==true && (bus[pixel] >= 0xF0 && bus[pixel + 1] >= 0xF0 && bus[pixel + 2] >= 0xF0)) )
							pinta=0;
						else
							pinta=1;
						matriz_resultado[j/scale][ i/scale] = pinta;
				}
				pixel+=3*scale;
			}
			pixel+=(96*3)*(scale-1);
		}		
	}
}
void ZoomIn (uint8_t scale, uint8_t cor, bool im) { //da zoom 2 vezes
	//im=0->airplane; im=1->bus
	uint32_t i, j, k, l;
	uint32_t pixel = 0;
	
	if(64*scale<128){
		limpa_tela();
	}
	
	for(i = 0; i < 64*scale; i+=scale){ // Linha (x)
		for(j = 0; j < 96*scale ; j+=scale){ // Coluna (y)
			if(j<128 && i<128){
				if(cor==1){
						if((im==false && (airplane[pixel] >= 0xF0 && airplane[pixel + 1] >= 0xF0 && airplane[pixel + 2] >= 0xF0)) ||
							 (im==true && (bus[pixel] >= 0xF0 && bus[pixel + 1] >= 0xF0 && bus[pixel + 2] >= 0xF0)) )
							GrContextForegroundSet(&sContext, ClrWhite);
						else
							GrContextForegroundSet(&sContext, ClrBlack);
						for(k=0; k<scale; k++){
							for(l=0; l<scale; l++){
								draw_pixel(j+l, i+k);
							}
						}
				}
				else{
					if((im==false && (airplane[pixel] >= 0xF0 && airplane[pixel + 1] >= 0xF0 && airplane[pixel + 2] >= 0xF0)) ||
						 (im==true && (bus[pixel] >= 0xF0 && bus[pixel + 1] >= 0xF0 && bus[pixel + 2] >= 0xF0)) )
						GrContextForegroundSet(&sContext, ClrBlack);
					else
						GrContextForegroundSet(&sContext, ClrWhite);
					for(k=0; k<scale; k++){
						for(l=0; l<scale; l++){
							draw_pixel(j+l, i+k);
						}
					}	
				}
			}
			pixel+=3;
		}
	}
}
void ZoomOut(uint8_t scale, uint8_t cor, bool im) {
	//im=0->airplane; im=1->bus
	uint8_t i, j;
	uint32_t pixel = 0;
	
	limpa_tela();
	
	
	for(i = 0; i < 64 ; i+=scale){ // Linha (x)
		for(j = 0; j < 96 ; j+=scale){ // Coluna (y)
			if(j<128 && i<128){
				if(cor==1){
					if((im==false && (airplane[pixel] >= 0xF0 && airplane[pixel + 1] >= 0xF0 && airplane[pixel + 2] >= 0xF0)) ||
							 (im==true && (bus[pixel] >= 0xF0 && bus[pixel + 1] >= 0xF0 && bus[pixel + 2] >= 0xF0)) )
						GrContextForegroundSet(&sContext, ClrWhite);
					else
						GrContextForegroundSet(&sContext, ClrBlack);
					draw_pixel(j/scale, i/scale);	
				}
				else{
					if((im==false && (airplane[pixel] >= 0xF0 && airplane[pixel + 1] >= 0xF0 && airplane[pixel + 2] >= 0xF0)) ||
							 (im==true && (bus[pixel] >= 0xF0 && bus[pixel + 1] >= 0xF0 && bus[pixel + 2] >= 0xF0)) )
						GrContextForegroundSet(&sContext, ClrBlack);
					else
						GrContextForegroundSet(&sContext, ClrWhite);
					draw_pixel(j/scale, i/scale);
				}
			}
			pixel+=3*scale;
		}
		pixel+=(96*3)*(scale-1);
	}
}


void Zoom (uint8_t scale, uint8_t cor, bool image_choose, bool zoom_choose){
	/*
	* Função que dá zoom na imagem escolhida através da variável image_choose
	* O zoom é definido entre aumentativo (In) ou diminutivo (Out) através do zoom_choose.
	* A inversão de cores é definido pela variável cor.
	* O nível de zoom é definido através de scale.
	*/
	
/*----------------------------------------------------------------------------
 * Definição de variáveis internas.
 *----------------------------------------------------------------------------*/
	uint32_t i, j, k, l;	//	Para movimentação dentro dos FOR.
	uint32_t pixel = 0;	// Para movimentação dentro da matriz da imagem.
	uint8_t pinta =0;
	
	limpaMatriz(cor);
	
	if(zoom_choose == false) // zoom_in
	{
		if(64 * scale < 128)
		{
			limpa_tela();
		}
		
		for(i = 0; i < 64*scale; i+=scale){ // Linha (x)
			for(j = 0; j < 96*scale ; j+=scale){ // Coluna (y)
				if(j<128 && i<128){
						if((image_choose==false && (airplane[pixel] >= 0xF0 && airplane[pixel + 1] >= 0xF0 && airplane[pixel + 2] >= 0xF0)) ||
							 (image_choose==true && (bus[pixel] >= 0xF0 && bus[pixel + 1] >= 0xF0 && bus[pixel + 2] >= 0xF0)) )
							pinta=0;
						else
							pinta=1;
						/*
						* FOR PARA IMPRESSÃO A SER REMOVIDO POSTERIORMENTE.
						*/
						for(k=0; k<scale; k++){
							for(l=0; l<scale; l++){
								if( (j+l) < 128 && (i+k) < 128)
									matriz_resultado[j+l][i+k] = pinta;
								//draw_pixel(j+l, i+k);
							}
						}
				}
				pixel+=3;
			}
		}		
	}
	else	// zoom_out
	{
		/*
		* FUNÇÃO LIMPA TELA A SER REMOVIDA POSTERIORMENTE.
		*/
		limpa_tela();
		
		for(i = 0; i < 64 ; i+=scale){ // Linha (x)
			for(j = 0; j < 96 ; j+=scale){ // Coluna (y)
				if(j<128 && i<128){
						if((image_choose==false && (airplane[pixel] >= 0xF0 && airplane[pixel + 1] >= 0xF0 && airplane[pixel + 2] >= 0xF0)) ||
								 (image_choose==true && (bus[pixel] >= 0xF0 && bus[pixel + 1] >= 0xF0 && bus[pixel + 2] >= 0xF0)) )
							pinta=0;
						else
							pinta=1;
						matriz_resultado[j/scale][ i/scale] = pinta;
				}
				pixel+=3*scale;
			}
			pixel+=(96*3)*(scale-1);
		}		
	}
}


void ZoomController (uint8_t cor, bool im, uint8_t it){
	//Zoom (uint8_t cor, bool im, uint8_t it, bool op)
	//op==false -> ZoomIn; op==true-> ZoomOut
		
	limpaMatriz(cor);
	limpa_tela();
	if(it==0){
		ZoomAssembly (1, im, false);
	}
	else if(it==1){
		ZoomAssembly (2, im, false);
	}
	else if(it==2){
		ZoomAssembly (4, im, false);
	}
	else if(it==3){
		ZoomAssembly (8, im, false);
	}
	else if(it==4){
		ZoomAssembly (4, im, false);
	}
	else if(it==5){
		ZoomAssembly (2, im, false);
	}
	else if(it==6){
		ZoomAssembly (1, im, false);
	}
	else if(it==7){
		ZoomAssembly (2, im, true);
	}
	else if(it==8){
		ZoomAssembly (4, im, true);
	}
	else if(it==9){
		ZoomAssembly (8, im, true);
	}
	else if(it==10){
		ZoomAssembly (4, im, true);
	}
	else if(it==11){
		ZoomAssembly (2, im, true);
	}
	SysTick_Wait1ms(500);
	
	MostraTela(cor);	
}
