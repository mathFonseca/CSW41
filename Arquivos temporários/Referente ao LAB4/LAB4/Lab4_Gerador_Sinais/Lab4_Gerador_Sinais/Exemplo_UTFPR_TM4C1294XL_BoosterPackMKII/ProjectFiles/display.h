#ifndef __PWM_H_
#define __PWM_H_
#include "cmsis_os.h"
#include <stdbool.h>
#include "grlib/grlib.h"
#include "cfaf128x128x16.h"

//uint8_t posicao_tempo_antiga[128][2];
//uint8_t posicao_eixo_x_antigo;

void initScreen(void);
void drawEixo();
void drawFunction(uint8_t tensao_ini,uint8_t tensao_fim,uint8_t posicao_tempo);
void som(void);
void drawFreqAmp(char* freq, char* amp);

#endif

