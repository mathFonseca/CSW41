#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "TM4C129.h" // Device header
#include "grlib/grlib.h"
#include "cfaf128x128x16.h"
#include "led.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h" 
#include "driverlib/pin_map.h"
#include "driverlib/uart.h" 
#include "utils/uartstdio.h"
#include "draw.h"

#define GPIO_PORTA_BASE         0x40004000  // GPIO Port A

// Variaveis display
tContext sContext;

void UART0_Handler (void)
{
	
	
	
}

// Inicialização UART
void inicia_UART(){
  uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                              SYSCTL_OSC_MAIN |
                                              SYSCTL_USE_PLL |
                                              SYSCTL_CFG_VCO_480),
                                              24000000); // PLL em 24MHz

    // Habilia o periferico da UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    // Inicia as configuracoes para o port A
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    // Delay de inicialização
    SysCtlDelay(3);
    // Configura PA0 como RX
    // Configura PA1 como Tx
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    
    // Delay de inicialização
    SysCtlDelay(3);
    
		GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    
    // Configuracoes do UART
    UARTStdioConfig(0, 9600, 24000000);
    // Envio de mensagem para a UART
    UARTprintf("Hello world!\n");
    // UARTprintf("\033[2J");
		
		while(1);
}

// ---- Utilizado para seed do rand
void inicia_DWT(){
/* DWT (Data Watchpoint and Trace) registers, only exists on ARM Cortex with a DWT unit */
/*!< DWT Control register */
#define KIN1_DWT_CONTROL             (*((volatile uint32_t*)0xE0001000))

/*!< CYCCNTENA bit in DWT_CONTROL register */
#define KIN1_DWT_CYCCNTENA_BIT       (1UL<<0)

/*!< DWT Cycle Counter register */
#define KIN1_DWT_CYCCNT              (*((volatile uint32_t*)0xE0001004))

/*!< DEMCR: Debug Exception and Monitor Control Register */
#define KIN1_DEMCR                   (*((volatile uint32_t*)0xE000EDFC))

/*!< Trace enable bit in DEMCR register */
#define KIN1_TRCENA_BIT              (1UL<<24)

/*!< TRCENA: Enable trace and debug block DEMCR (Debug Exception and Monitor Control Register */	
#define KIN1_InitCycleCounter() \
	KIN1_DEMCR |= KIN1_TRCENA_BIT

/*!< Reset cycle counter */
#define KIN1_ResetCycleCounter() \
	KIN1_DWT_CYCCNT = 0

/*!< Enable cycle counter */
#define KIN1_EnableCycleCounter() \
	KIN1_DWT_CONTROL |= KIN1_DWT_CYCCNTENA_BIT

/*!< Disable cycle counter */
#define KIN1_DisableCycleCounter() \
	KIN1_DWT_CONTROL &= ~KIN1_DWT_CYCCNTENA_BIT	

/*!< Read cycle counter register */
#define KIN1_GetCycleCounter() \
	KIN1_DWT_CYCCNT
}

// Função inicia perifericos	
// ---- Inicializa os perifericos
void init_all(){
	cfaf128x128x16Init();
	led_init();

}



// Criação das threads 
void uart							(void const *argument);
void trata_informacao (void const *argument);
void pwm							(void const *argument);
void escreve_display  (void const *argument);

// Variável que determina ID das threads
osThreadId uart_ID; 
osThreadId trata_informacao_ID; 
osThreadId pwm_ID; 
osThreadId escreve_display_ID; 
 
// Definição das threads
osThreadDef (uart, osPriorityAboveNormal, 1, 0);     // thread object
osThreadDef (trata_informacao, osPriorityNormal, 1, 0);     // thread object
osThreadDef (pwm, osPriorityNormal, 1, 0);     // thread object
osThreadDef (escreve_display, osPriorityNormal, 1, 0);     // thread object

// Criação das threads
void uart(void const *argument){






}



void escreve_display(void const *argument){}

void pwm(void const *argument){}
	
void trata_informacao(void const *argument){}

// Inicializa as threads
int Init_Thread (void) {
	
	escreve_display_ID = osThreadCreate (osThread(escreve_display), NULL);
	if (!escreve_display_ID) return(-1);
	
	uart_ID = osThreadCreate (osThread(uart), NULL);
	if (!uart_ID) return(-1);
	
	pwm_ID= osThreadCreate (osThread(pwm), NULL);
	if (!pwm_ID) return(-1);
	
	escreve_display_ID = osThreadCreate (osThread(escreve_display), NULL);
	if (!escreve_display_ID ) return(-1);

	return(0);
}





int main() {
	
	inicia_UART();
	
	// Inicia contador de ciclos
	#ifndef GANTT
	inicia_DWT();
	
	KIN1_InitCycleCounter(); /* enable DWT hardware */
	KIN1_ResetCycleCounter(); /* reset cycle counter */
	KIN1_EnableCycleCounter(); /* start counting */
	#endif
		
	// Inicializa o Kernel
	osKernelInitialize();	

#ifndef GANTT	
	//Initializing all peripherals
	init_all();
#endif
	
	// Inicializa as Threads
	if(Init_Thread()==-1)
		return 0;
		// Mensagens de erro de inicialização
	
	// Inicializa o Kernel, junto com as threads
	osKernelStart();
	
	//Main aguarda para sempre
	osDelay(osWaitForever);

}
