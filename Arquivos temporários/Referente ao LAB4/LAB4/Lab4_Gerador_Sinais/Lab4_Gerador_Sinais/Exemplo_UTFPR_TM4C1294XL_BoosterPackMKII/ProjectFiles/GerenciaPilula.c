#include "GerenciaPilula.h"

//Informações: Parede lateral: 3px - Parede Superior: 2px

 
//-Função que cria e armazena as pílulas-----------------------------------------------------
//São 8 linhas e o padrão está nas colunas, ou seja, as todas as linhas ímpares e pares são iguais.
//O tratamento acontece da seguinte forma: o programa analisa se a linha q está sendo tratada é par ou ímpar devido ao padrão identificado
//Se PAR: ele trata em 3 blocos: o primeiro bloco são as 6 primeiras pílulas, o segundo bloco são as 4 pílulas do meio(considerando que o bloco com 6 pixels na verdade são 2 pílulas), incluindo uma maior de 6 pixels e em seguida as últimas 6 pílulas
//Se ÍMPAR: ele trata em 4 blocos, que seguem o mesmo padrão explicado acima, entretando ele printa, 3 pílulas, 5 pílulas, 5 pílulas e 3 pílulas.
void Inicia_pilulas(struct pilula TodasPilulas[8][16]){
	//pilula TodasPilulas[8][16]; //Vamos considerar 8 linhas por 16 colunas e esta será a matriz que armazenará as Pílulas

	int conta_linha = 1;
	int Num_pilula  = 1;				//Número da pílula que estou tratando

	int valor_linha = 6;				//Todas as linhas começam alinhadas com a lateral direita em uma distância de 6 pixels

	int Total=0;				//Variável que conta as colunas

	for(conta_linha=1;conta_linha<=8;conta_linha++){		//percorre as linhas, aumentando de 8 em 8 até a última fileira
		int valor_coluna = 6; //O valor da coluna deve ser reiniciado à cada linha

		if(conta_linha%2 != 0){ 			//Trata as linhas ímpares
			
			//Tratando as 6 primeiras pílulas
			for(Num_pilula = 1; Num_pilula <=6; Num_pilula ++){ //Preenche as 6 primeiras colunas
				
				TodasPilulas[conta_linha-1][Num_pilula -1].coluna = valor_coluna; 
				TodasPilulas[conta_linha-1][Num_pilula -1].linha = valor_linha;
				TodasPilulas[conta_linha-1][Num_pilula -1].ativo = 1;

				valor_coluna+=7;		//O valor da coluna cresce de 7 em 7 para o primeiro bloco
				Total++;
			}
			

			valor_coluna+=7;				//Pula 7 linhas para chegar à linha desejada
			//Tratando as 4 pílulas do meio
			for(Num_pilula=7;Num_pilula <=10;Num_pilula++){
				TodasPilulas[conta_linha-1][Num_pilula -1].coluna = valor_coluna;  				
				TodasPilulas[conta_linha-1][Num_pilula -1].linha = valor_linha; 				
				TodasPilulas[conta_linha-1][Num_pilula -1].ativo = 1;

				if(Num_pilula == 8){	//Não vai somar 7 pílulas se as pílulas a número 8
					valor_coluna+=3;		
				}
				else
					valor_coluna+=7;

			}		

			valor_coluna+=7;				//Pula 7 linhas para chegar à linha desejada
			
			//Tratando as 6 últimas pílulas
			for(Num_pilula =11;Num_pilula <=16;Num_pilula++){
				TodasPilulas[conta_linha-1][Num_pilula -1].coluna = valor_coluna;  				
				TodasPilulas[conta_linha-1][Num_pilula -1].linha = valor_linha; 				
				TodasPilulas[conta_linha-1][Num_pilula -1].ativo = 1;

				valor_coluna+=7;		//O valor da coluna cresce de 7 em 7 para o primeiro bloco
			}	
		}
		else{ 						//Trata as linhas pares

			//Tratando as 3 primeiras pílulas
			for(Num_pilula = 1; Num_pilula <=3; Num_pilula ++){ //Preenche as 6 primeiras colunas
				TodasPilulas[conta_linha-1][Num_pilula -1].coluna = valor_coluna;  				
				TodasPilulas[conta_linha-1][Num_pilula -1].linha = valor_linha; 				
				TodasPilulas[conta_linha-1][Num_pilula -1].ativo = 1;

				valor_coluna+=7;		//O valor da coluna cresce de 7 em 7 para o primeiro bloco
			}

			valor_coluna+=5;			//Soma 6 colunas para iniciar a próxima sequência de pílulas


			//Tratando as 10 primeiras pílulas
			for(Num_pilula = 4; Num_pilula <=13; Num_pilula ++){ //Preenche as 6 primeiras colunas
				
				if(conta_linha == 4 && (Num_pilula == 8 || Num_pilula == 9)){			//Trata da linha especial onde existe um quadrado e não coloca na matriz a 2 pílulas

					TodasPilulas[conta_linha-1][Num_pilula -1].coluna = 0;  				
					TodasPilulas[conta_linha-1][Num_pilula -1].linha = 0; 				
					TodasPilulas[conta_linha-1][Num_pilula -1].ativo = 0;

					valor_coluna+=7;		//O valor da coluna cresce de 7 em 7 para o primeiro bloco
				}
				else{
					TodasPilulas[conta_linha-1][Num_pilula -1].coluna = valor_coluna;  				
					TodasPilulas[conta_linha-1][Num_pilula -1].linha = valor_linha; 				
					TodasPilulas[conta_linha-1][Num_pilula -1].ativo = 1;

					valor_coluna+=7;		//O valor da coluna cresce de 7 em 7 para o primeiro bloco
				}
			}


			valor_coluna+=5;			//Soma 5 + 4 = 9 colunas para iniciar a próxima sequência de pílulas

			//Tratando as 3 primeiras pílulas
			for(Num_pilula = 14; Num_pilula <=16; Num_pilula ++){ //Preenche as 6 primeiras colunas
				TodasPilulas[conta_linha-1][Num_pilula -1].coluna = valor_coluna;  				
				TodasPilulas[conta_linha-1][Num_pilula -1].linha = valor_linha; 				
				TodasPilulas[conta_linha-1][Num_pilula -1].ativo = 1;

				valor_coluna+=7;		//O valor da coluna cresce de 7 em 7 para o primeiro bloco
			}

		}

		valor_linha+=8;					//A linha cresce de 8 em 8 pixels
		
	}

}


