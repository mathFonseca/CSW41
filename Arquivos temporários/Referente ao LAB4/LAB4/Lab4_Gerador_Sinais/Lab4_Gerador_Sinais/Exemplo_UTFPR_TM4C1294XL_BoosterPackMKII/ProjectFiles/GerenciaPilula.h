#include <stdio.h>

//Definindo o Tipo Pilula
typedef struct pilula{
	//Atributos de p�lula - cada p�lula tem o tamanho de 1x3
	int coluna;
	int linha;
	int ativo; //Esta flag determina se a p�lula deve ou n�o ser printada
}pilula;

void Inicia_pilulas(struct pilula TodasPilulas[8][16]);
