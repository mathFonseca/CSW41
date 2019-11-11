#include <stdio.h>

//Definindo o Tipo Vitamina
typedef struct vitamina{
	//Atributos de Vitamina - cada vitamina tem o tamanho de 3x3
	int coluna;
	int linha;
	int ativo; //Esta flag determina se a pílula deve ou não ser printada
} vitamina;

void Inicia_vitamina(struct vitamina TodasVitaminas[2][2]);

