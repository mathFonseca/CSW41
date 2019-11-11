#include "UART_m.h"

// Inicializao UART
void UART_Enable(void){
		uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000); // PLL em 120MHz
		static unsigned long g_ulBase = 0;
		static const unsigned long g_ulUARTBase[3] = {UART0_BASE, UART1_BASE, UART2_BASE};
		static const unsigned long g_ulUARTPeriph[3] = {SYSCTL_PERIPH_UART0, SYSCTL_PERIPH_UART1, SYSCTL_PERIPH_UART2};
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); // Habilita o periferico da UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // Inicia as configuracoes para o port A
    SysCtlDelay(3); // Delay de inicializao
    GPIOPinConfigure(GPIO_PA0_U0RX); // Configura PA0 como RX
    GPIOPinConfigure(GPIO_PA1_U0TX); // Configura PA1 como Tx
    SysCtlDelay(3); // Delay de inicializao
		GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    g_ulBase = g_ulUARTBase[0]; // Select the base address of the UART.
    MAP_SysCtlPeripheralEnable(g_ulUARTPeriph[0]);// Enable the UART peripheral for use.
    MAP_UARTConfigSetExpClk(g_ulBase, ui32SysClock, 115200, (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE | UART_CONFIG_WLEN_8)); // Configure the UART for 115200, n, 8, 1
    MAP_UARTEnable(g_ulBase); // Enable the UART operation.
		UART0 ->	CTL &= ~(1<<0); //Disable a UART, para fazer as configuracoes 
		UART0 ->	LCRH = (0x3<<5); //palavra de 8 bits sem paridade 
		UART0 ->	IFLS |= (1<<4); //Uso default
		UART0 ->	IM |= (1<<4); //Habilita interrupcoes 
		UART0 ->	CTL = (1<<0)|(1<<8)|(1<<9);	//Habilita a UART com Rx&Tx
		NVIC -> ISER[0] |= (1<<5); //Habilita interrupcoes de UART0 no vetor de interrupcoes
		__NVIC_SetPriority(UART0_IRQn,1<<5);
    // Configuracoes do UART
    SysCtlDelay(30); // Delay de inicializao
    UARTprintString("\033[2J"); // Envio de mensagem para a UART
    UARTprintMenu(); // Envio de mensagem para a UART
}

/*Leitura de elementos*/
char UARTreceive(void){//leitura de caractere
	char c;
	while((UART0->FR & (1<<4)) != 0); // verifica se esta receber
	c = UART0->DR; //Recebe caracteres para a saida
	return c;
}

void UARTsend(char c){//Envio UM caracter
	while((UART0->FR & (1<<5)) != 0);
	UART0->DR = c;	//Escreve na saida o conteudo de c
}

void UARTprintString(char* string){//Envia uma String
	while(*string)//enquanto houver elementos na string
		UARTsend(*(string++));//Envia caracteres para a saida
}

// Menu inicial
void UARTprintMenu(){
	UARTprintString("FORMA DE ONDA:\n\r");
	UARTprintString("	1 -> Senoidal\n\r");
	UARTprintString("	2 -> Quadrada\n\r");
	UARTprintString("	3 -> Dente de serra\n\r");
	UARTprintString("	4 -> Triangular\n\r");
	UARTprintString("	5 -> Trapezoidal\n\r");
	UARTprintString("CONFIGURACAO DO SINAL:\n\r");
	UARTprintString("	A -> +5Hz | a -> +1Hz\n\r");
	UARTprintString("	S -> -5Hz | s -> -1Hz\n\r");
	UARTprintString("	J -> +1V  | j -> +0.1V\n\r");
	UARTprintString("	K -> -1V  | k -> -0.1V\n\r");
}
