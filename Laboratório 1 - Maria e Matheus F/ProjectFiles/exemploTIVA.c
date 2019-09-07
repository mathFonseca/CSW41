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
#include "cfaf128x128x16.h"
#include "buttons.h"
#include "joy.h"

//To print on the screen
//tContext sContext;
extern void f_asm(int dir, uint8_t *img);
extern void Zoom (uint8_t scale, uint8_t cor, bool image_choose, bool zoom_choose);
extern void ZoomController (uint8_t cor, bool im, uint8_t it);
extern void init_functions(void);
void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);


void init_all(void){
	cfaf128x128x16Init();
	button_init();
	init_functions();
	SysTick_Init();
}

bool button_read_debounce_s1(void) {
	uint8_t i = 0;
	while(i < 50) {
		if(button_read_s1())
			i++;
		else
			return false;
	}
	return true;
}
bool button_read_debounce_s2(void) {
	uint8_t i = 0;
	while(i < 50) {
		if(button_read_s2())
			i++;
		else
			return false;
	}
	return true;
}

int main (void) {
	bool s1_press=false, s2_press =false;
	uint8_t cor=1;
	bool im=false;
	
	uint8_t it=0;
	
	init_all();
	//init_sidelong_menu();
	while(1)
	{
			s1_press = button_read_debounce_s1();
			if(s1_press) { 	//troca de cor
					if(cor == 1) 
						cor = 0;
					else 
						cor = 1;
			}
			s2_press = button_read_debounce_s2();
			if(s2_press) {
				if(im){	
					im = false;
				}
				else{		
					im = true;						
				}
			}
			
			ZoomController(cor, im, it);
			it++;
			if(it==13){
				it=0;
			}
	}
}

