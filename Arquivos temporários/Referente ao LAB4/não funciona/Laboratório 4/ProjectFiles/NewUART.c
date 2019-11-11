#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include "grlib/grlib.h"
#include "NewUART.h"

void init_uart(){
	SYSCTL-> RCGCUART |= (1<<0); 					//habilita uso da uart0
	SYSCTL-> RCGCGPIO |= (1<<0); 					// habilita a porta A do GPIO
	GPIOA_AHB -> AFSEL = (1<<1)|(1<<0); 	// habilita a funcao especial do GPIO
	GPIOA_AHB -> PCTL = (1<<0)|(1<<4);
	GPIOA_AHB -> DEN = (1<<0)|(1<<1);
	UART0 ->	CTL &= ~(1<<0); 						//desabilita a uart
	UART0 ->	IBRD = 65; 									// baudrate
	UART0 ->	FBRD = 7;
	UART0 ->	LCRH = (0x3<<5); 						//palavra de 8 bits
	UART0 ->	CC	= 0x0; 									// usa clock do sistema
	UART0 ->	IFLS |=(1<<4); 
	UART0 ->	IM |=(1<<4); 								//habilita interrupcao
	UART0 ->	CTL = (1<<0)|(1<<8)|(1<<9); //habilita a uart
	NVIC -> ISER[0] |= (1<<5); 						//habilita interrupcao da uart
	__NVIC_SetPriority(UART0_IRQn,1<<5);
}

/*Envio de elementos*/
void print_char(char c){
	while((UART0->FR & (1<<5)) != 0);
	UART0->DR = c;
}
void uart_print_string(char * string){
	while(*string){
		print_char(*(string++));
	}
}
/*Leitura de elementos*/
char read_char(void){
	char c;
	while((UART0->FR & (1<<4)) != 0);
	c = UART0->DR;
	return c;
}