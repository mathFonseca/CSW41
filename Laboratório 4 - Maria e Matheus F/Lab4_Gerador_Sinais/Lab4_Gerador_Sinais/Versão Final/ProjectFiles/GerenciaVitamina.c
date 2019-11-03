#include "GerenciaVitamina.h"

//Informações: Parede lateral: 3px - Parede Superior: 2px

 
//-Função que cria e armazena as pílulas-----------------------------------------------------
//São 8 linhas e o padrão está nas colunas, ou seja, as todas as linhas ímpares e pares são iguais.
//O tratamento acontece da seguinte forma: o programa analisa se a linha q está sendo tratada é par ou ímpar devido ao padrão identificado
//Se PAR: ele trata em 3 blocos: o primeiro bloco são as 6 primeiras pílulas, o segundo bloco são as 4 pílulas do meio(considerando que o bloco com 6 pixels na verdade são 2 pílulas), incluindo uma maior de 6 pixels e em seguida as últimas 6 pílulas
//Se ÍMPAR: ele trata em 4 blocos, que seguem o mesmo padrão explicado acima, entretando ele printa, 3 pílulas, 5 pílulas, 5 pílulas e 3 pílulas.
void Inicia_vitamina(struct vitamina TodasVitaminas[2][2]){

	//Cria estaticamente as vitaminas:

	//Primeira Vitamina - posição do centro:
	TodasVitaminas[0][0].coluna = 6;
	TodasVitaminas[0][0].linha = 9;
	TodasVitaminas[0][0].ativo = 1;

	//Segunda Vitamina - posição do centro:
	TodasVitaminas[0][1].coluna = 121;
	TodasVitaminas[0][1].linha = 9;
	TodasVitaminas[0][1].ativo = 1;

	//Terceira Vitamina - posição do centro:
	TodasVitaminas[1][0].coluna = 6;
	TodasVitaminas[1][0].linha = 57;
	TodasVitaminas[1][0].ativo = 1;

	//Quarta Vitamina - posição do centro:
	TodasVitaminas[1][1].coluna = 121;
	TodasVitaminas[1][1].linha = 57;
	TodasVitaminas[1][1].ativo = 1;

}

