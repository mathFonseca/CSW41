#ifndef UART_H
#define UART_H

extern void init_uart(void);
extern void print_char(char c);
extern void uart_print_string(char * string);
extern char read_char(void);

#endif
