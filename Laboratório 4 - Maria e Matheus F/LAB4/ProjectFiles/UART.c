// uart.c
// Desenvolvido para a placa EK-TM4C1294XL

// Bibliotecas
#include <stdint.h>
#include "TM4C129.h"                    // Device header

#define SYSCTL_RCGCUART_R       (*((volatile uint32_t *)0x400FE618))
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRUART_R         (*((volatile uint32_t *)0x400FEA18))
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
#define GPIO_PORTA_AHB_DEN_R    (*((volatile uint32_t *)0x4005851C))
#define GPIO_PORTA_AHB_AFSEL_R  (*((volatile uint32_t *)0x40058420))
#define GPIO_PORTA_AHB_PCTL_R   (*((volatile uint32_t *)0x4005852C))
#define GPIO_PORTA_AHB_AMSEL_R  (*((volatile uint32_t *)0x40058528))


#define UART0_CTL_R             (*((volatile uint32_t *)0x4000C030))
#define UART0_IBRD_R            (*((volatile uint32_t *)0x4000C024))
#define UART0_FBRD_R            (*((volatile uint32_t *)0x4000C028))
#define UART0_LCRH_R            (*((volatile uint32_t *)0x4000C02C))
#define UART0_CC_R              (*((volatile uint32_t *)0x4000CFC8))
#define UART0_FR_R              (*((volatile uint32_t *)0x4000C018))
#define UART0_DR_R              (*((volatile uint32_t *)0x4000C000))	
#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define GPIO_PORT_A							0x0001

// Defini��es de Fun��es
void UART_Send(uint8_t data);
void UART_Send_String(const uint8_t mensagem[]);

// Vari�veis Globais
const uint8_t st_inicio[] = "UART Iniciada com sucesso.\n\r\0";
uint32_t interruption;

// -------------------------------------------------------------------------------
// Fun��o UART_Init
// Inicializa as configura��es b�sicas da UART
// Par�metro de entrada: N�o tem.
// Par�metro de sa�da: N�o tem
void UART_Init(void)
{
	SYSCTL_RCGCUART_R = 0x01;            
  while((SYSCTL_PRUART_R & (0x01) ) != (0x01)){};
	
	SYSCTL_RCGCGPIO_R |= GPIO_PORT_A;
	while((SYSCTL_PRGPIO_R & (GPIO_PORT_A) ) != (GPIO_PORT_A) ){};
	
	GPIO_PORTA_AHB_AMSEL_R &= 0xFC;
	GPIO_PORTA_AHB_PCTL_R = (GPIO_PORTA_AHB_PCTL_R & 0xFFFFFF00) + 0x00000011;
	GPIO_PORTA_AHB_AFSEL_R |= 0x03;  // Fun��o alternativa em PA1 e PA0
	GPIO_PORTA_AHB_DEN_R |= 0x03;
		
		
  UART0_CTL_R = UART0_CTL_R & 0xFFFFFFFE;      
  UART0_IBRD_R = 520;                    
  UART0_FBRD_R = 53;                     
                                        
  UART0_LCRH_R = 0x78;
	UART0_CC_R = 0;
  UART0_CTL_R = UART0_CTL_R | 0x301;
		
	UART_Send_String(st_inicio);	
}

// -------------------------------------------------------------------------------
// Fun��o UART_Send_String
// Envia, pela UART, uma string.
// Par�metro de entrada: Uma string de char.
// Par�metro de sa�da: N�o tem
void UART_Send_String(const uint8_t mensagem[])
{
	uint32_t i = 0;
	while(mensagem[i] != '\0' && !interruption) //  trocar 1 por !interruption
	{
		UART_Send(mensagem[i]);
		// Necess�rio esperar por 1 milisegundo aqui.
		i++;
	}
}

// -------------------------------------------------------------------------------
// Fun��o UART_Send
// Envia pela UART um �nico char
// Par�metro de entrada: Um char
// Par�metro de sa�da: N�o tem
void UART_Send(uint8_t data)
{
	while((UART0_FR_R & UART_FR_TXFF)!=0 && 1);//  trocar 1 por !interruption
	UART0_DR_R = data;
}


// -------------------------------------------------------------------------------
// Fun��o UART_Rcv
// Recebe um char da UART
// Par�metro de entrada: Uma string de char.
// Par�metro de sa�da: N�o tem
uint8_t UART_Rcv(void)
{
	uint32_t exit_flag = 1;
	while((UART0_FR_R & UART_FR_RXFE)!=0 && exit_flag)
	{
		if(interruption) // Interrup��o durante leitura. Trocar 1 por um Signal. (Interrup��o)
		{
			exit_flag = 0; // Sai do while.
		}
	}
	return (uint8_t)(UART0_DR_R & 0xFF);
}
