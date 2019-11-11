#include <stdio.h>
#include <math.h>

int primos[50];

int mensagem[300] = {5,10,9,9,0,1,0,0,6,6,6,7,15,14,15,15,7,6,9,9,0,1,0,0,6,8,6,7,15,14,15,15,7,11,9,9,0,1,0,0,6,1,6,7,15,14,15,15,2,7,9,9,0,1,0,0,2,6,6,7,15,14,15,15,2,7,9,9,0,1,0,0,4,12,6,7,15,14,15,15,6,8,9,9,0,1,0,0,6,7,6,7,15,14,15,15,7,11,9,9,0,1,0,0,5,10,6,7,15,14,15,15,7,5,9,9,0,1,0,0,5,10,6,7,15,14,15,15,2,7,9,9,0,1,0,0,5,15,6,7,15,14,15,15,6,12,9,9,0,1,0,0,5,10,6,7,15,14,15,15,7,11,9,9,0,1,0,0,2,7,6,7,15,14,15,15,2,7,9,9,0,1,0,0,4,11,6,7,15,14,15,15,7,6,9,9,0,1,0,0,5,11,6,7,15,14,15,15,2,7,9,9,0,1,0,0,4,13,6,7,15,14,15,15,6,15,9,9,0,1,0,0,6,8,6,7,15,14,15,15,7,4,9,9,0,1,0,0,5,10,6,7,15,14,15,15,7,10,9,9,0,1,0,0,8,10,6,5,0,2,0,0,8,14,11,15,15,14,15,15};

int mensagem_calculada[100];

int tabela,i;
int vetor[100];

int isPrime(int number){

    if(number < 2) return 0;
    if(number == 2) return 1;
    if(number % 2 == 0) return 0;
    for(int i=3; (i*i)<=number; i+=2){
        if(number % i == 0 ) return 0;
    }
    return 1;
}


int calcula_chave(){
	int piso,teto;
	int j=0,i=0;
		
	piso = mensagem[j+2]*16*16*16 + mensagem[j+3]*16*16 + mensagem[j+4]*16*16*16*16*16 + mensagem[j+5]*16*16*16*16 + mensagem[j+6]*16*16*16*16*16*16*16 + mensagem[j+7]*16*16*16*16*16*16; 
	teto = piso + 255;

	printf("%d e %d\n",piso,teto);

	while(teto != piso){
		if(isPrime(teto--)){
			primos[i++]=teto+1;
			printf("primo:%d\n",teto+1);	
		}
	}
	return 104711;

}

void calcula_mensagem(){
	int j=0;
	
	for(int i=0;i<40;i++){
		mensagem_calculada[i] = (mensagem[j]*16 + mensagem[j+1]*1 + mensagem[j+2]*16*16*16 + mensagem[j+3]*16*16 + mensagem[j+4]*16*16*16*16*16 + mensagem[j+5]*16*16*16*16 + mensagem[j+6]*16*16*16*16*16*16*16 + mensagem[j+7]*16*16*16*16*16*16);
		j+=8;
		printf("%d\n",mensagem_calculada[i]);	
	}
}

void check_pen(int chave){
	
	if(3*chave/2 == mensagem_calculada[35])
		return 1;
	else
		return 0;

};


void check_ult(){
};


int main()
{
	calcula_mensagem();
	int chave = calcula_chave();
	int valor =0;


	int i=0;

	for(i=0;i<34;i++){
		if(i%2){
			valor = mensagem_calculada[i]+chave;
			printf("%c",valor);
		}
		else{
			valor = mensagem_calculada[i]-chave;
			printf("%c",valor);
		}
		valor = 0;
	}


}
