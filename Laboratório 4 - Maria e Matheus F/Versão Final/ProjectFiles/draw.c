#include "draw.h"

// Definicao dos mutex
osMutexDef(desenhar);	
osMutexId(desenhar_ID);

bool pisca_vitamina = false;
char pontos_char[10];
bool sentido_pacman_direita = true;
const unsigned char *pacman;
const unsigned char pacman_length;
const unsigned char *fantasma_imagem;
const unsigned char fantasma_length;

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

void desenho_mapa(tContext sContext){
	int32_t coluna,linha;
	int32_t index = 1;
	
	// Declaracao do mutex ID
	osMutexWait(desenhar_ID,osWaitForever);

	
	#ifndef GANTT
	
	for(linha=1;linha<=LINHA_LENGTH;linha++){
		for(coluna=1;coluna<=COLUNA_LENGTH;coluna++){
			
			if(mapa[index++]<TRESHOLD)
				GrContextForegroundSet(&sContext, COR_PAREDE_MAPA);
			else	
				GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
			
			GrPixelDraw(&sContext,coluna,linha);
			
		}
	}

	// Printa linha do painel de instrumentos
	GrContextForegroundSet(&sContext, COR_PAINEL);
	
	for(linha=1;linha<=PAINEL_LINHA_LENGTH;linha++){
		for(coluna=1;coluna<=PAINEL_COLUNA_LENGTH;coluna++){
			
			//Printa linha verde na tela	
			GrPixelDraw(&sContext,PAINEL_X_INICIAL+coluna,PAINEL_Y_INICIAL+linha);
		}
	}
	
	#endif
	
	osMutexRelease(desenhar_ID);
};
	

void desenho_fantasma(tContext sContext,Fantasma *fantasma,uint32_t cor,int8_t imagem_fantasma, bool colisao){
	uint32_t coluna,linha;
	uint32_t index = 1;
	
	desenhar_ID = osMutexCreate(osMutex(desenhar));

		// Seleciona a imagem com base no imagem_fantasma acima
	if(imagem_fantasma == 1)
		fantasma_imagem = fantasma2_imagem;
	else
	if(imagem_fantasma == 2)
		fantasma_imagem = fantasma4_imagem;
	else
	if(imagem_fantasma == 3)
		fantasma_imagem = fantasma2_imagem;
	else
	if(imagem_fantasma == 4)
		fantasma_imagem = fantasma4_imagem;
	else
	if(imagem_fantasma == 5)
		fantasma_imagem = fantasma_morto;
	
	#ifndef GANTT
	// Seta para a limpeza do rastro
	GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
	GrContextBackgroundSet(&sContext, COR_FUNDO_MAPA);
	#endif

		if(fantasma->colisao_fantasma == false){
		fantasma->colisao_fantasma = false;
		// Cima - Limpa baixo
		if(fantasma->fantasma_dir == 0){
			for(coluna=1;coluna<=FANTASMA_COLUNA_LENGTH;coluna++){
					
					#ifndef GANTT
						GrPixelDraw(&sContext,fantasma->pos_x_fantasma+coluna,fantasma->pos_y_fantasma+FANTASMA_LINHA_LENGTH+1);
					#endif
			}
		}
		else	
		// Direita - Limpa esquerda
		if(fantasma->fantasma_dir == 1){
			for(linha=1;linha<=FANTASMA_LINHA_LENGTH;linha++){
					#ifndef GANTT
						GrPixelDraw(&sContext,fantasma->pos_x_fantasma,linha+fantasma->pos_y_fantasma);
					#endif
			}	
		}
		else
		// Baixo - Limpa cima
		if(fantasma->fantasma_dir == 2){
			for(coluna=1;coluna<=FANTASMA_COLUNA_LENGTH;coluna++){
				
				#ifndef GANTT
					GrPixelDraw(&sContext,coluna+fantasma->pos_x_fantasma,fantasma->pos_y_fantasma);
				#endif
			}
		}
		else
		// Esquerda - Limpa direita
		if(fantasma->fantasma_dir == 3){
			for(linha=1;linha<=FANTASMA_LINHA_LENGTH;linha++){
					#ifndef GANTT
						GrPixelDraw(&sContext,fantasma->pos_x_fantasma+FANTASMA_COLUNA_LENGTH+1,linha+fantasma->pos_y_fantasma);
					#endif
			}	
		}
	}

// Mostra o fantasma na tela
	if(imagem_fantasma == 5)
		index = 0;
	else
		index = 1;
	
	for(linha=1;linha<=FANTASMA_LINHA_LENGTH;linha++){
		for(coluna=1;coluna<=FANTASMA_COLUNA_LENGTH;coluna++){
				
			#ifndef GANTT
			if(fantasma_imagem[index++] > TRESHOLD)
				GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
			else
				GrContextForegroundSet(&sContext, cor);
			
			osMutexWait(desenhar_ID,osWaitForever);
			GrPixelDraw(&sContext,coluna+fantasma->pos_x_fantasma,linha+fantasma->pos_y_fantasma);
			osMutexRelease(desenhar_ID);
			#endif
		
		}
	}
	
}


void desenho_pacman(tContext sContext,uint32_t pos_x_pacman,uint32_t pos_y_pacman,uint8_t pacman_dir,int8_t imagem_pacman){
	int32_t coluna,linha;
	uint32_t index = 1;
	
	// Seleciona a imagem com base no imagem_pacman acima
	if(imagem_pacman == 1)
		pacman = pacman1;
	else
	if(imagem_pacman == 2)
		pacman = pacman2;
	else
	if(imagem_pacman == 3)
		pacman = pacman3;
	
	// Verifica a direcao do pacman
	if(pacman_dir == 1)
		sentido_pacman_direita = true;
	else
	if (pacman_dir == 3)
		sentido_pacman_direita = false;
	
	#ifndef GANTT
	// Seta para a limpeza do rastro
	GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
	#endif
	
	// Cima - Limpa baixo
	if(pacman_dir == 0){
		for(coluna=1;coluna<=PACMAN_COLUNA_LENGTH;coluna++){
				
				#ifndef GANTT
					GrPixelDraw(&sContext,pos_x_pacman+coluna,pos_y_pacman+PACMAN_LINHA_LENGTH+1);
					#endif
		}
	}
	else
	// Direita - Limpa esquerda
	if(pacman_dir == 1){
		for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
				#ifndef GANTT
					GrPixelDraw(&sContext,pos_x_pacman,linha+pos_y_pacman);
				#endif
		}	
	}
	else
	// Baixo - Limpa cima
	if(pacman_dir == 2){
		for(coluna=1;coluna<=PACMAN_COLUNA_LENGTH;coluna++){
			
			#ifndef GANTT
				GrPixelDraw(&sContext,coluna+pos_x_pacman,pos_y_pacman);
			#endif
		}
	}
	else
	// Esquerda - Limpa direita
	if(pacman_dir == 3){
		for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
				#ifndef GANTT
					GrPixelDraw(&sContext,pos_x_pacman+PACMAN_COLUNA_LENGTH+1,linha+pos_y_pacman);
				#endif
		}	
	}
	
	
	// IMPRIMIR O PACMAN 
	// VERIFICA DIRECAO
	
index = 1;
	if(sentido_pacman_direita == true){
		for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
			for(coluna=1;coluna<=PACMAN_COLUNA_LENGTH;coluna++){
					
				#ifndef GANTT
				if(pacman[index++]>TRESHOLD)
					GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
				else
					GrContextForegroundSet(&sContext, COR_PACMAN);
				
				osMutexWait(desenhar_ID,osWaitForever);
				GrPixelDraw(&sContext,coluna+pos_x_pacman,linha+pos_y_pacman);
				osMutexRelease(desenhar_ID);					
				
				#endif
			}
		}
	}
	else
	if(sentido_pacman_direita == false){
		for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
			for(coluna=PACMAN_COLUNA_LENGTH;coluna>=1;coluna--){
					
				#ifndef GANTT
				if(pacman[index++]>TRESHOLD)
					GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
				else
					GrContextForegroundSet(&sContext, COR_PACMAN);
					
				GrPixelDraw(&sContext,coluna+pos_x_pacman,linha+pos_y_pacman);
					
					#endif
			}
		}
	}
	else
	if(sentido_pacman_direita == true && (pacman_dir == 0 || pacman_dir == 2)){
		for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
			for(coluna=1;coluna<=PACMAN_COLUNA_LENGTH;coluna++){
					
				#ifndef GANTT
				if(pacman[index++]>TRESHOLD)
					GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
				else
					GrContextForegroundSet(&sContext, COR_PACMAN);
					
				GrPixelDraw(&sContext,coluna+pos_x_pacman,linha+pos_y_pacman);
					
					#endif
			}
		}
	}
	else
	if(sentido_pacman_direita == false && (pacman_dir == 0 || pacman_dir == 2)){
		for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
			for(coluna=PACMAN_COLUNA_LENGTH;coluna>=1;coluna--){
					
				#ifndef GANTT
				if(pacman[index++]>TRESHOLD)
					GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
				else
					GrContextForegroundSet(&sContext, COR_PACMAN);
					
				GrPixelDraw(&sContext,coluna+pos_x_pacman,linha+pos_y_pacman);
					
					#endif
			}
		}
	}

}

void desenho_pilulas(tContext sContext,struct pilula TodasPilulas[8][16]){
	uint32_t coluna,linha;
	
	osMutexWait(desenhar_ID,osWaitForever);

	#ifndef GANTT
	GrContextForegroundSet(&sContext, COR_PILULA);
	
	for(linha = 0;linha<8;linha++){
        for(coluna = 0; coluna < 16;coluna++){
            if(TodasPilulas[linha][coluna].ativo){
								GrLineDraw(&sContext,TodasPilulas[linha][coluna].coluna,TodasPilulas[linha][coluna].linha,TodasPilulas[linha][coluna].coluna+2,TodasPilulas[linha][coluna].linha);
            }
        }
    }
	#endif
		
	osMutexRelease(desenhar_ID);
}

void desenho_vitaminas(tContext sContext,struct vitamina TodasVitaminas[2][2]){
	uint32_t coluna,linha;
	
	osMutexWait(desenhar_ID,osWaitForever);
	
	#ifndef GANTT
	if(pisca_vitamina){
		pisca_vitamina = !pisca_vitamina;
		GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
	}
	else{
		pisca_vitamina = !pisca_vitamina;
		GrContextForegroundSet(&sContext, COR_VITAMINA);
	}
	for(linha = 0;linha<2;linha++){
        for(coluna = 0; coluna < 2;coluna++){
            if(TodasVitaminas[linha][coluna].ativo){
							GrLineDraw(&sContext,TodasVitaminas[linha][coluna].coluna,TodasVitaminas[linha][coluna].linha,TodasVitaminas[linha][coluna].coluna+2,TodasVitaminas[linha][coluna].linha);
							GrLineDraw(&sContext,TodasVitaminas[linha][coluna].coluna,TodasVitaminas[linha][coluna].linha+1,TodasVitaminas[linha][coluna].coluna+2,TodasVitaminas[linha][coluna].linha+1);
							GrLineDraw(&sContext,TodasVitaminas[linha][coluna].coluna,TodasVitaminas[linha][coluna].linha+2,TodasVitaminas[linha][coluna].coluna+2,TodasVitaminas[linha][coluna].linha+2);            }
        }
    }
	#endif
		
	osMutexRelease(desenhar_ID);
}

void desenho_painel(tContext sContext,uint16_t pontuacao,uint8_t vidas){
	int8_t index = 3;
	int32_t coluna,linha;
	osMutexWait(desenhar_ID,osWaitForever);	
	
	// Valor inteiro para string
	intToString(pontuacao,pontos_char,3,10,0);
	
	#ifndef GANTT
		// Pontuacao 
		GrContextBackgroundSet(&sContext, COR_PAINEL);
		GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
		GrStringDraw(&sContext,pontos_char,3,72,72,1);
		
		// Vidas

	GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
	
		while(index--){
			GrLineDraw(&sContext,X_VIDA_INICIAL+10*index,Y_VIDA_INICIAL,X_VIDA_INICIAL+3+10*index,Y_VIDA_INICIAL);
			GrLineDraw(&sContext,X_VIDA_INICIAL+10*index,Y_VIDA_INICIAL+1,X_VIDA_INICIAL+3+10*index,Y_VIDA_INICIAL+1);
			GrLineDraw(&sContext,X_VIDA_INICIAL+10*index,Y_VIDA_INICIAL+2,X_VIDA_INICIAL+3+10*index,Y_VIDA_INICIAL+2);
		}		
		GrContextForegroundSet(&sContext, ClrLightGreen);
	
		while(vidas--){
			GrLineDraw(&sContext,X_VIDA_INICIAL+10*vidas,Y_VIDA_INICIAL,X_VIDA_INICIAL+3+10*vidas,Y_VIDA_INICIAL);
			GrLineDraw(&sContext,X_VIDA_INICIAL+10*vidas,Y_VIDA_INICIAL+1,X_VIDA_INICIAL+3+10*vidas,Y_VIDA_INICIAL+1);
			GrLineDraw(&sContext,X_VIDA_INICIAL+10*vidas,Y_VIDA_INICIAL+2,X_VIDA_INICIAL+3+10*vidas,Y_VIDA_INICIAL+2);
		}
	
	#endif
	
	osMutexRelease(desenhar_ID);
}
void limpar_desenho_pacman(tContext sContext, uint32_t pos_x_pacman,uint32_t pos_y_pacman){
	int32_t coluna,linha;
	uint32_t index = 1;
	
	osMutexWait(desenhar_ID,osWaitForever);

	#ifndef GANTT
				GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
	#endif
	
		for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
			for(coluna=1;coluna<=PACMAN_COLUNA_LENGTH;coluna++){
					
				#ifndef GANTT
				
				GrPixelDraw(&sContext,coluna+pos_x_pacman,linha+pos_y_pacman);
					
					#endif
			}
	}
	
	osMutexRelease(desenhar_ID);
}
void desenho_pacman_morrendo1(tContext sContext,int32_t pos_x_pacman, int32_t pos_y_pacman){
	uint32_t coluna,linha;
	uint32_t index = 1;
	
	osMutexWait(desenhar_ID,osWaitForever);
	
	for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
		for(coluna=PACMAN_COLUNA_LENGTH;coluna>=1;coluna--){
				
			#ifndef GANTT
			if(pacman1_morrendo[index++]>TRESHOLD)
				GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
			else
				GrContextForegroundSet(&sContext, COR_PACMAN);
				
			GrPixelDraw(&sContext,coluna+pos_x_pacman,linha+pos_y_pacman);
				
				#endif
		}
	}
	
	osMutexRelease(desenhar_ID);
}
void desenho_pacman_morrendo2(tContext sContext,int32_t pos_x_pacman, int32_t pos_y_pacman){	
	uint32_t coluna,linha;
	uint32_t index = 1;

	osMutexWait(desenhar_ID,osWaitForever);

	for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
		for(coluna=PACMAN_COLUNA_LENGTH;coluna>=1;coluna--){
				
			#ifndef GANTT
			if(pacman2_morrendo[index++]>TRESHOLD)
				GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
			else
				GrContextForegroundSet(&sContext, COR_PACMAN);
				
			GrPixelDraw(&sContext,coluna+pos_x_pacman,linha+pos_y_pacman);
				
				#endif
		}
	}
	osMutexRelease(desenhar_ID);
}

void desenho_pacman_morrendo3(tContext sContext,int32_t pos_x_pacman, int32_t pos_y_pacman){	
	uint32_t coluna,linha;
	uint32_t index = 1;
	
	osMutexWait(desenhar_ID,osWaitForever);
	
	for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
		for(coluna=PACMAN_COLUNA_LENGTH;coluna>=1;coluna--){
				
			#ifndef GANTT
			if(pacman3_morrendo[index++]>TRESHOLD)
				GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
			else
				GrContextForegroundSet(&sContext, COR_PACMAN);
				
			GrPixelDraw(&sContext,coluna+pos_x_pacman,linha+pos_y_pacman);
				
				#endif
		}
	}	
	osMutexRelease(desenhar_ID);
}
void desenho_pacman_morrendo4(tContext sContext,int32_t pos_x_pacman, int32_t pos_y_pacman){	
	uint32_t coluna,linha;
	uint32_t index = 1;
	
	osMutexWait(desenhar_ID,osWaitForever);
	
	for(linha=1;linha<=PACMAN_LINHA_LENGTH;linha++){
		for(coluna=PACMAN_COLUNA_LENGTH;coluna>=1;coluna--){
				
			#ifndef GANTT
			if(pacman4_morrendo[index++]>TRESHOLD)
				GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
			else
				GrContextForegroundSet(&sContext, COR_PACMAN);
				
			GrPixelDraw(&sContext,coluna+pos_x_pacman,linha+pos_y_pacman);
				
				#endif
		}
	}
	
	osMutexRelease(desenhar_ID);
}
void escreve_frase(tContext sContext){
	#ifndef GANTT
	GrStringDraw(&sContext,"Start - botao 1",17,PAINEL_X_INICIAL+2,PAINEL_Y_INICIAL+2,0);
	#endif
}
void escreve_frase_final(tContext sContext,uint16_t pontuacao){
	uint16_t linha, coluna;
	#ifndef GANTT
	// Printa linha do painel de instrumentos
	GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
	GrContextBackgroundSet(&sContext, COR_PAINEL);
	
	GrStringDraw(&sContext,"Pontos",6,PAINEL_X_INICIAL+2,PAINEL_Y_INICIAL+2,0);
	GrStringDraw(&sContext,pontos_char,3,72,72,1);
	
	GrContextForegroundSet(&sContext, COR_PAINEL);
	
	for(linha=1;linha<=PAINEL_LINHA_LENGTH;linha++){
		for(coluna=1;coluna<=PAINEL_COLUNA_LENGTH;coluna++){
			
			//Printa linha verde na tela	
			GrPixelDraw(&sContext,PAINEL_X_INICIAL+coluna,PAINEL_LINHA_LENGTH + PAINEL_Y_INICIAL + 1 + linha);
		}
	}
	
	GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);
	GrContextBackgroundSet(&sContext, COR_PAINEL);
	GrStringDraw(&sContext,"Restart - botao 1",20,PAINEL_X_INICIAL+2,PAINEL_Y_INICIAL+PAINEL_LINHA_LENGTH+2,0);
	#endif
}
void limpa_tela(tContext sContext){
	uint16_t linha, coluna;

	#ifndef GANTT
	osMutexWait(desenhar_ID,osWaitForever);

	GrContextBackgroundSet(&sContext, COR_FUNDO_MAPA);
	GrContextForegroundSet(&sContext, COR_FUNDO_MAPA);

	for(linha=0;linha<=128;linha++){
		for(coluna=0;coluna<=128;coluna++){
			
			//Printa linha verde na tela	
			GrPixelDraw(&sContext,coluna,linha);
		}
	}

	osMutexRelease(desenhar_ID);
	#endif
}


