#ifndef __UART_H__
#define __UART_H__

void uart_init(void);
void uart_putc(uint8_t data);
uint8_t uart_getc(void);

#endif
