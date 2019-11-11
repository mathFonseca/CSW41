#include "UART.h"

// Inicializao UART
void UART_Enable(void){
		uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000); // PLL em 120MHz
		static unsigned long g_ulBase = 0;
		static const unsigned long g_ulUARTBase[3] = {UART0_BASE, UART1_BASE, UART2_BASE};
		static const unsigned long g_ulUARTPeriph[3] = {SYSCTL_PERIPH_UART0, SYSCTL_PERIPH_UART1, SYSCTL_PERIPH_UART2};
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);		// Habilita o periferico da UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); 		// Inicia as configuracoes para o port A
    SysCtlDelay(3); 																// Delay de inicializao
    GPIOPinConfigure(GPIO_PA0_U0RX); 								// Configura PA0 como RX
    GPIOPinConfigure(GPIO_PA1_U0TX); 								// Configura PA1 como Tx
    SysCtlDelay(3); 																// Delay de inicializao
		GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    g_ulBase = g_ulUARTBase[0]; 										// Select the base address of the UART.
    MAP_SysCtlPeripheralEnable(g_ulUARTPeriph[0]);// Enable the UART peripheral for use.
    MAP_UARTConfigSetExpClk(g_ulBase, ui32SysClock, 115200, (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE | UART_CONFIG_WLEN_8)); // Configure the UART for 115200, n, 8, 1
    MAP_UARTEnable(g_ulBase); 											// Enable the UART operation.
		UART0 ->	CTL &= ~(1<<0); 											//Disable a UART, para fazer as configuracoes 
		UART0 ->	LCRH = (0x3<<5); 											//palavra de 8 bits sem paridade 
		UART0 ->	IFLS |= (1<<4); 											//Uso default
		UART0 ->	IM |= (1<<4); 												//Habilita interrupcoes 
		UART0 ->	CTL = (1<<0)|(1<<8)|(1<<9);						//Habilita a UART com Rx&Tx
		NVIC -> ISER[0] |= (1<<5); 											//Habilita interrupcoes de UART0 no vetor de interrupcoes
		__NVIC_SetPriority(UART0_IRQn,1<<5);
    SysCtlDelay(30); 																// Delay de inicializao
    UARTprintString("\033[2J"); 										// Envio de mensagem para a UART
		UARTprintString("\033[0;0H");
}

/*Leitura da uart*/
char UARTreceive(void){
	char c;
	while((UART0->FR & (1<<4)) != 0);									// verifica se esta receber
	c = UART0->DR; 																		//Recebe caracteres para a saida
	return c;
}

/* Envia um char */
void UARTsend(char c){
	while((UART0->FR & (1<<5)) != 0);
	UART0->DR = c;	//Escreve na saida o conteudo de c
}

/* Envia uma string */
void UARTprintString(char* string){//Envia uma String
	while(*string)//enquanto houver elementos na string
		UARTsend(*(string++));//Envia caracteres para a saida
}


// Menu da uart
void UARTprintMenu(int i){
	
	if(i == 0)
	{
		UARTprintString("Seleciona a forma de onda:\n\r");
		UARTprintString("	0 -> Sine\n\r");
		UARTprintString("	1 -> Triangle\n\r");	
		UARTprintString("	2 -> SawTooth\n\r");
		UARTprintString("	3 -> Square\n\r");
		UARTprintString("	4 -> Trapezoid\n\r");
	}
	else if(i == 1)
	{
		UARTprintString("Seleciona a frequência da onda:\n\r");
		UARTprintString("	0 -> 0Hz\n\r");
		UARTprintString("	1 -> 20Hz\n\r");	
		UARTprintString("	2 -> 50Hz\n\r");
		UARTprintString("	3 -> 100Hz\n\r");
		UARTprintString("	4 -> 150Hz\n\r");
		UARTprintString("	5 -> 200Hz\n\r");		
	}		
	else
	{
		UARTprintString("Selecione uma amplitude:\n\r");
		UARTprintString("	0 -> 0.5V\n\r");
		UARTprintString("	1 -> 1.0V\n\r");
		UARTprintString("	2 -> 1.5V\n\r");
		UARTprintString("	3 -> 2.0V\n\r");
		UARTprintString("	4 -> 2.5V\n\r");
		UARTprintString("	5 -> 3.0V\n\r");
		UARTprintString("	6 -> 3.3V\n\r");
	}
}


