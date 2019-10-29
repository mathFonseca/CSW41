#ifndef UART_H
#define UART_H

extern void UART_Init(void);
extern void UART_Send_String(const uint8_t mensagem[]);
extern void UART_Send(uint8_t data);
extern uint8_t UART_Rcv(void);


#endif /* UART_H */
