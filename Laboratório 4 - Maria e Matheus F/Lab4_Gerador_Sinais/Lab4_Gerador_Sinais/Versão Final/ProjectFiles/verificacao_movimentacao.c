#include "cer

// Verifica colisao fantasma
// ---- Indica a colisao fantasma parede

bool verifica_colisao_fantasma(Fantasma fantasma){

	/* MANIPULANDO MEMORIA */
	
	// Coleta posicoes de teste
	pos_teste =(fantasma.pos_x_fantasma)+COLUNA_LENGTH*fantasma.pos_y_fantasma;

	// Se o pacman deseja ir pra cima, verifica possibilidade
	// Se for menor que o treshold, significa que estou batendo num ponto preto
	if(fantasma.fantasma_dir == 0 && (mapa[pos_teste-COLUNA_LENGTH+1] < TRESHOLD || 
											mapa[pos_teste-COLUNA_LENGTH+2] < TRESHOLD || 
											mapa[pos_teste-COLUNA_LENGTH+3] < TRESHOLD ||
											mapa[pos_teste-COLUNA_LENGTH+4] < TRESHOLD || 
											mapa[pos_teste-COLUNA_LENGTH+5] < TRESHOLD || 
											mapa[pos_teste-COLUNA_LENGTH+6] < TRESHOLD || 
											mapa[pos_teste-COLUNA_LENGTH+7] < TRESHOLD)){
		return false;
	}
	else
	// Testa movimento para a direita
	if(fantasma.fantasma_dir == 1 && (mapa[pos_teste+PACMAN_COLUNA_LENGTH+1] < TRESHOLD ||
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+1*COLUNA_LENGTH+1] < TRESHOLD || 
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+2*COLUNA_LENGTH+1] < TRESHOLD ||
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+3*COLUNA_LENGTH+1] < TRESHOLD ||
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+4*COLUNA_LENGTH+1] < TRESHOLD ||
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+5*COLUNA_LENGTH+1] < TRESHOLD ||
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+6*COLUNA_LENGTH+1] < TRESHOLD)){
		return false;
	}
	else
	// Testa movimento para baixo
	if(fantasma.fantasma_dir == 2 && (mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+1] < TRESHOLD || 
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+2] < TRESHOLD ||
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+3] < TRESHOLD ||
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+4] < TRESHOLD ||
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+5] < TRESHOLD ||
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+6] < TRESHOLD ||
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+7] < TRESHOLD )){
		return false;
	}
	else
	// Testa movimento para esquerda
	if(fantasma.fantasma_dir == 3 && (mapa[pos_teste+0*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+1*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+2*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+3*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+4*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+5*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+6*COLUNA_LENGTH] < TRESHOLD )){
		return false;
	}
	
	
	// Se falhar em todos os casos, confirma a possibilidade de movimentacao	
		return true;
}

// Movimenta pacman
// ---- Executa a movimentacao do pacman
void movimenta_pacman(){
			
		// Cima
		if(pacman_dir == CIMA){
			pos_y_pacman--;
		}
		else
		// Direita
		if(pacman_dir == DIREITA){
			pos_x_pacman++;
		}
		else
		// Baixo
		if(pacman_dir == BAIXO){
			pos_y_pacman++;
		}
		else
		// Esquerda
		if(pacman_dir == ESQUERDA){
			pos_x_pacman--;
		}
		else{
			pos_x_pacman_teste = pos_x_pacman;
			pos_y_pacman_teste = pos_y_pacman;
		}

}

// Verifica colisao pacman
// ---- Indica a colisao pacman paredes
bool verifica_colisao_pacman(){

	/* MANIPULANDO MEMORIA */
	
	// Coleta posicoes de teste
	pos_teste =(pos_x_pacman)+COLUNA_LENGTH*pos_y_pacman;

	// Se o pacman deseja ir pra cima, verifica possibilidade
	// Se for menor que o treshold, significa que estou batendo num ponto preto
	if(dir_joy == 0 && (mapa[pos_teste-COLUNA_LENGTH+1] < TRESHOLD || 
											mapa[pos_teste-COLUNA_LENGTH+2] < TRESHOLD || 
											mapa[pos_teste-COLUNA_LENGTH+3] < TRESHOLD ||
											mapa[pos_teste-COLUNA_LENGTH+4] < TRESHOLD || 
											mapa[pos_teste-COLUNA_LENGTH+5] < TRESHOLD || 
											mapa[pos_teste-COLUNA_LENGTH+6] < TRESHOLD || 
											mapa[pos_teste-COLUNA_LENGTH+7] < TRESHOLD)){
		dir_joy = pacman_dir;
		return false;
	}
	else
	// Testa movimento para a direita
	if(dir_joy == 1 && (mapa[pos_teste+PACMAN_COLUNA_LENGTH+1] < TRESHOLD ||
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+1*COLUNA_LENGTH+1] < TRESHOLD || 
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+2*COLUNA_LENGTH+1] < TRESHOLD ||
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+3*COLUNA_LENGTH+1] < TRESHOLD ||
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+4*COLUNA_LENGTH+1] < TRESHOLD ||
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+5*COLUNA_LENGTH+1] < TRESHOLD ||
											mapa[pos_teste+PACMAN_COLUNA_LENGTH+6*COLUNA_LENGTH+1] < TRESHOLD)){
		dir_joy = pacman_dir;
		return false;
	}
	else
	// Testa movimento para baixo
	if(dir_joy == 2 && (mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+1] < TRESHOLD || 
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+2] < TRESHOLD ||
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+3] < TRESHOLD ||
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+4] < TRESHOLD ||
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+5] < TRESHOLD ||
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+6] < TRESHOLD ||
											mapa[pos_teste+PACMAN_LINHA_LENGTH*COLUNA_LENGTH+7] < TRESHOLD )){
		dir_joy = pacman_dir;
		return false;
	}
	else
	// Testa movimento para esquerda
	if(dir_joy == 3 && (mapa[pos_teste+0*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+1*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+2*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+3*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+4*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+5*COLUNA_LENGTH] < TRESHOLD || 
											mapa[pos_teste+6*COLUNA_LENGTH] < TRESHOLD )){
		dir_joy = pacman_dir;
		return false;
	}
	
	
	// Se falhar em todos os casos, confirma a possibilidade de movimentacao	
		pacman_dir = dir_joy;
		return true;
}


// Verifica colisao Vitamina
// ---- Indica a colisao pacman Vitamina
bool verifica_colisao_vitamina(uint32_t vitamina_x,uint32_t vitamina_y){
	
	// Colisao baixo
	if( pos_y_pacman+PACMAN_LINHA_LENGTH/2 < vitamina_y+3 && pos_y_pacman+PACMAN_LINHA_LENGTH/2 > vitamina_y+3)
		if ( pos_x_pacman+PACMAN_LINHA_LENGTH/2 > vitamina_x-2 && pos_x_pacman+PACMAN_LINHA_LENGTH/2 < vitamina_x+3)
			return true;
	
	// Colisao cima
	if( pos_y_pacman+PACMAN_LINHA_LENGTH/2 > vitamina_y && pos_y_pacman+PACMAN_LINHA_LENGTH/2 < vitamina_y+3)
		if ( pos_x_pacman+PACMAN_LINHA_LENGTH/2 > vitamina_x-2 && pos_x_pacman+PACMAN_LINHA_LENGTH/2 < vitamina_x+3)
			return true;	
	
	// Em caso de não encontrar
	return false;

}

// Verifica colisao pilula
// ---- Indica a colisao pacman pilula
bool verifica_colisao_pilula(uint32_t pilula_x,uint32_t pilula_y){
	
	// Colisao vertical
	if(pilula_y > pos_y_pacman && pilula_y < pos_y_pacman+PACMAN_LINHA_LENGTH){
		if (pilula_x > pos_x_pacman && pilula_x+2 < pos_x_pacman+PACMAN_COLUNA_LENGTH)
			return true;
	}
	else
	// Colisao horizontal
	if (pilula_x > pos_x_pacman && pilula_x+2 < pos_x_pacman+PACMAN_COLUNA_LENGTH){	
		if(pilula_y > pos_y_pacman && pilula_y < pos_y_pacman+PACMAN_LINHA_LENGTH)
			return true;
	}
	else
		
	// Em caso de não encontrar
	return false;

}



// Movimenta fantasma
// ---- Executa a movimentacao do fantasma
void movimenta_fantasma(Fantasma* fantasma){
			
		// Cima
		if(fantasma->fantasma_dir == CIMA){
			fantasma->pos_y_fantasma--;
		}
		else
		// Direita
		if(fantasma->fantasma_dir == DIREITA){
			fantasma->pos_x_fantasma++;
		}
		else
		// Baixo
		if(fantasma->fantasma_dir == BAIXO){
			fantasma->pos_y_fantasma++;
		}
		else
		// Esquerda
		if(fantasma->fantasma_dir == ESQUERDA){
			fantasma->pos_x_fantasma--;
		}
		else{
			fantasma->pos_x_fantasma_teste = fantasma->pos_x_fantasma;
			fantasma->pos_y_fantasma_teste = fantasma->pos_y_fantasma;
		}

}
	
 
// Cacada ao pacman
// ---- Indica a movimentacao necessaria para alcancar o pacman
void cacada_pacman(Fantasma* fantasma){

	// Seguir na horizontal
	if(pos_x_pacman > fantasma->pos_x_fantasma){
		fantasma->fantasma_dir = DIREITA;
	}
	else
	if(pos_x_pacman < fantasma->pos_x_fantasma){
		fantasma->fantasma_dir = ESQUERDA;
	}
	else
	// Seguir na vertical
	if(pos_y_pacman > fantasma->pos_y_fantasma){
		fantasma->fantasma_dir = BAIXO;
	}
	else
	if(pos_y_pacman < fantasma->pos_y_fantasma){
		fantasma->fantasma_dir = CIMA;
	}

}
// Cacada ao pacman PREVISAO
// ---- Indica a movimentacao necessaria para alcancar uma possivel futura posicao do pacman
bool proximidade_pacman(Fantasma* fantasma){

	// Se o pacman esta proximo em altura 
	if(abs(pos_y_pacman - fantasma->pos_y_fantasma) < LIMITE_PROXIMIDADE_PC_FANTASMA)
		return false;
	else
	if(abs(pos_x_pacman - fantasma->pos_x_fantasma) < LIMITE_PROXIMIDADE_PC_FANTASMA)
		return false;
	else
		return true;
}

// Checa colisao pacman-fantasma
// ---- Indica colisao apos realizadas as movimentacoes
bool colisao_fantasma_pacman(Fantasma fantasma){

	// Colisao vertical - Fantasma embaixo do pacman
	if((pos_x_pacman + PACMAN_LINHA_LENGTH/2 > fantasma.pos_x_fantasma &&
			pos_x_pacman + PACMAN_LINHA_LENGTH/2 < fantasma.pos_x_fantasma+FANTASMA_COLUNA_LENGTH) &&
			 (pos_y_pacman + PACMAN_LINHA_LENGTH == fantasma.pos_y_fantasma ||
			  pos_y_pacman == fantasma.pos_y_fantasma + FANTASMA_LINHA_LENGTH)){
			 
		return true;
	}
	else
	// Colisao horizontal - Fantasma do lado do pacman
	if((pos_y_pacman + PACMAN_COLUNA_LENGTH/2 >= fantasma.pos_y_fantasma &&
			pos_y_pacman <= fantasma.pos_y_fantasma + PACMAN_COLUNA_LENGTH/2) &&
			(pos_x_pacman + PACMAN_COLUNA_LENGTH == fantasma.pos_x_fantasma ||
			 pos_x_pacman == fantasma.pos_x_fantasma + FANTASMA_LINHA_LENGTH)){
			 
		return true;
	}

	return false;
}


// Inicia Variaveis
// ---- Inicia e reinicia variaveis dos personagens
void inicia_variaveis(){
	uint8_t index;
	
	// Pacman
	pos_x_pacman = X_INICIAL_PACMAN;
	pos_y_pacman = Y_INICIAL_PACMAN;
	dir_joy=5,pacman_dir=5; // 0 - cima,1 - direita, 2-baixo, 3-esquerda

	// Fantasma
	// Inicia os fantasmas
	//Fantasma1
	fantasma1.pos_x_fantasma = X_INICIAL_FANTASMA;
	fantasma1.pos_y_fantasma = Y_INICIAL_FANTASMA;
	//Fantasma2
	fantasma2.pos_x_fantasma = X_INICIAL_FANTASMA;
	fantasma2.pos_y_fantasma = Y_INICIAL_FANTASMA;
	//Fantasma3
	fantasma3.pos_x_fantasma = X_INICIAL_FANTASMA;
	fantasma3.pos_y_fantasma = Y_INICIAL_FANTASMA;
	
	// Inicializa variaveis de set de busca
	fantasma1.busca = true;
	fantasma2.busca = true;
	fantasma3.busca = true;
	
	// Ajusta vetor de possibilidades
	for (index = 0;index<3;index++){
		fantasma1.possibilidades[index]=0;
		fantasma2.possibilidades[index]=0;
		fantasma3.possibilidades[index]=0;
	}
	
	// Colocar direcoes aleatorias
	fantasma1.fantasma_dir = rand() % 3;
	fantasma2.fantasma_dir = rand() % 3;
	fantasma3.fantasma_dir = rand() % 3;

}