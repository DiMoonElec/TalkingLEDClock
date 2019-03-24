#include <stdint.h>
#include "stm32f1xx.h"
#include "uart.h"

/*
  UART2
  TX2 - PA2
  RX2 - PA3
    
    USARTx_TX - Alternate function push-pull
    USARTx_RX - Input pull-up
*/

void uart_init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;   //Включаем тактирование порта PA
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN; //Включаем тактирование USART2
  
  GPIOA->CRL &= ~((GPIO_CRL_CNF2 | GPIO_CRL_MODE2)
                | (GPIO_CRL_CNF3 | GPIO_CRL_MODE3));
  
  //USARTx_TX - PA2
  GPIOA->CRL |= (0x01 << GPIO_CRL_MODE2_Pos) 
              | (0x02 << GPIO_CRL_CNF2_Pos);
  
  //USARTx_RX - PA3
  GPIOA->CRL |= (0x02 << GPIO_CRL_CNF3_Pos);
  GPIOA->ODR |= 1<<3;
  
  
  //Настройка UART
  //19200
  USART2->BRR = (117 << USART_BRR_DIV_Mantissa_Pos)
    | (3 << USART_BRR_DIV_Fraction_Pos);
  
  //USART2->CR2 = 0;
  
  USART2->CR1 = USART_CR1_UE //uart enable
    | USART_CR1_TE //Transmitter is enabled
    | USART_CR1_RE; //Receiver is enabled
}

void uart_putc(uint8_t data)
{
  while (!(USART2->SR & USART_SR_TXE))
    ;
  USART2->DR = data;
}

uint8_t uart_getc(void)
{
  while (!(USART2->SR & USART_SR_RXNE))
    ;
  return USART2->DR;
}
