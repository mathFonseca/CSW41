#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os.h"
#include "TM4C129.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h" 
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "utils/uartstdio.h"

#define SYSCTL_PERIPH_UART0     0xf0001800  // UART 0
#define SYSCTL_PERIPH_UART1     0xf0001801  // UART 1
#define SYSCTL_PERIPH_UART2     0xf0001802  // UART 2
#define UART_CONFIG_WLEN_MASK   0x00000060  // Mask for extracting word length
#define UART_CONFIG_WLEN_8      0x00000060  // 8 bit data
#define UART_CONFIG_WLEN_7      0x00000040  // 7 bit data
#define UART_CONFIG_WLEN_6      0x00000020  // 6 bit data
#define UART_CONFIG_WLEN_5      0x00000000  // 5 bit data
#define UART_CONFIG_STOP_MASK   0x00000008  // Mask for extracting stop bits
#define UART_CONFIG_STOP_ONE    0x00000000  // One stop bit
#define UART_CONFIG_STOP_TWO    0x00000008  // Two stop bits
#define UART_CONFIG_PAR_MASK    0x00000086  // Mask for extracting parity
#define UART_CONFIG_PAR_NONE    0x00000000  // No parity
#define UART_CONFIG_PAR_EVEN    0x00000006  // Even parity
#define UART_CONFIG_PAR_ODD     0x00000002  // Odd parity
#define UART_CONFIG_PAR_ONE     0x00000082  // Parity bit is one
#define UART_CONFIG_PAR_ZERO    0x00000086  // Parity bit is zero
#define GPIO_PORTA_BASE         0x40004000  // GPIO Port A

void UARTenable();
void UARTsend(char c);
char UARTreceive(void);
void UARTprintString(char* string);
void UARTprintMenu();
