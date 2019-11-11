#include "cmsis_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "TM4C129.h" // Device header
#include "../grlib/grlib.h"
#include "cfaf128x128x16.h"
#include "driverlib/sysctl.h" // driverlib
#include "GerenciaPilula.h"
#include "GerenciaVitamina.h"

// Variaveis das imagens
// MAPA
extern const unsigned char mapa[];
extern const unsigned char mapa_length;	
// FANTASMA
extern const unsigned char fantasma1_imagem[];
extern const unsigned char fantasma_length1;
extern const unsigned char fantasma2_imagem[];
extern const unsigned char fantasma_length2;
extern const unsigned char fantasma3_imagem[];
extern const unsigned char fantasma_length3;
extern const unsigned char fantasma4_imagem[];
extern const unsigned char fantasma_length4;
extern const unsigned char fantasma_morto[];
extern const unsigned char fantasma_morto_length;
// PACMAN
extern const unsigned char pacman1[];
extern const unsigned char pacman1_length;
extern const unsigned char pacman2[];
extern const unsigned char pacman2_length;
extern const unsigned char pacman3[];
extern const unsigned char pacman3_length;
// PACMAN MORRENDO
extern const unsigned char pacman1_morrendo[];
extern const unsigned char pacman1_morrendo_length;
extern const unsigned char pacman2_morrendo[];
extern const unsigned char pacman2_morrendo_length;
extern const unsigned char pacman3_morrendo[];
extern const unsigned char pacman3_morrendo_length;
extern const unsigned char pacman4_morrendo[];
extern const unsigned char pacman4_morrendo_length;

// Variáveis do display
//tContext sContext;

//#define GANTT 1

/*----MAPA----*/
// Treshold de cor dos pixels
#define TRESHOLD 128
#define LINHA_LENGTH 67
#define COLUNA_LENGTH 128
#define COR_FUNDO_MAPA ClrBlack
#define COR_PAREDE_MAPA ClrBlue

/*----FANTASMA----*/
// Tamanho fantasma
#define FANTASMA_LINHA_LENGTH 7
#define FANTASMA_COLUNA_LENGTH 7
#define X_INICIAL_FANTASMA 61
#define Y_INICIAL_FANTASMA 26
// Cores dos fantasmas
// Fantasma 1 - Vermelho
#define COR_FANTASMA1 ClrRed
// Fantasma 2 - Verde
#define COR_FANTASMA2 ClrLightGreen
// Fantasma 3 - Rosa
#define COR_FANTASMA3 ClrLightPink
// Fantasma Fugindo 
#define COR_FUGA ClrLightGrey

typedef struct fantasma{
	// Struct do fantasma
	int32_t pos_x_fantasma_teste,pos_y_fantasma_teste;

	// Posicao atual do fantasma
	int32_t pos_x_fantasma;
	int32_t pos_y_fantasma;
	
	// Direcao do fantasma
	int8_t fantasma_dir;
	
	// Posicoes possiveis
	int8_t possibilidades[4];
	
	// Estados de busca e guarda
	bool busca;
	bool vivo;
	bool fugir;
	bool colisao_fantasma;

} Fantasma;


/*----PACMAN----*/
// Tamanho pacman
#define PACMAN_LINHA_LENGTH 7
#define PACMAN_COLUNA_LENGTH 7
#define X_INICIAL_PACMAN 60
#define Y_INICIAL_PACMAN 50
#define COR_PACMAN ClrYellow

/*----PAINEL----*/
// Tamanho fantasma
#define PAINEL_LINHA_LENGTH 18
#define PAINEL_COLUNA_LENGTH 128
#define PAINEL_X_INICIAL 0
#define PAINEL_Y_INICIAL 70
#define COR_PAINEL ClrDarkGreen
#define X_VIDA_INICIAL 10
#define Y_VIDA_INICIAL 95

/*----PILULA----*/
#define COR_PILULA ClrOrange

/*----VITAMINA----*/
#define COR_VITAMINA ClrCyan

static void intToString(int64_t value, char * pBuf, uint32_t len, uint32_t base, uint8_t zeros);
static void floatToString(float value, char *pBuf, uint32_t len, uint32_t base, uint8_t zeros, uint8_t precision);
void desenho_mapa(tContext sContext);
void desenho_pacman(tContext sContext, uint32_t pos_x_pacman,uint32_t pos_y_pacman,uint8_t pacman_dir,int8_t imagem_pacman);
void desenho_fantasma(tContext sContext,Fantasma *fantasma,uint32_t cor,int8_t imagem_fantasma, bool colisao);
void desenho_pilulas(tContext sContext,struct pilula TodasPilulas[8][16]);
void desenho_vitaminas(tContext sContext,struct vitamina TodasVitaminas[2][2]);
void desenho_painel(tContext sContext,uint16_t pontuacao,uint8_t vidas);
void limpar_desenho_pacman(tContext sContext, uint32_t pos_x_pacman,uint32_t pos_y_pacman);
void desenho_pacman_morrendo1(tContext sContext,int32_t pos_x_pacman, int32_t pos_y_pacman);
void desenho_pacman_morrendo2(tContext sContext,int32_t pos_x_pacman, int32_t pos_y_pacman);
void desenho_pacman_morrendo3(tContext sContext,int32_t pos_x_pacman, int32_t pos_y_pacman);
void desenho_pacman_morrendo4(tContext sContext,int32_t pos_x_pacman, int32_t pos_y_pacman);
void escreve_frase(tContext sContext);
void escreve_frase_final(tContext sContext,uint16_t pontuacao);
void limpa_tela(tContext sContext);
