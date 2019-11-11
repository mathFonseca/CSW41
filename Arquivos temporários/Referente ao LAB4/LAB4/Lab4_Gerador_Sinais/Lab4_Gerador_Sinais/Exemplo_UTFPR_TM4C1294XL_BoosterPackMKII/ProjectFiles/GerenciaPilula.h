#include <stdio.h>

//Definindo o Tipo Pilula
typedef struct pilula{
	//Atributos de pílula - cada pílula tem o tamanho de 1x3
	int coluna;
	int linha;
	int ativo; //Esta flag determina se a pílula deve ou não ser printada
}pilula;

void Inicia_pilulas(struct pilula TodasPilulas[8][16]);
