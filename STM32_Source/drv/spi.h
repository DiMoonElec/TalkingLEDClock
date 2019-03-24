#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>

void SPI1_Init(void);
uint16_t SPI1_Transfer(uint16_t data);
void SPI1_DMARead(void *data, uint16_t len, void (*CallBack)(void));

/*
void SPI1_Init(void);
void SPI1_TxDMA(void *data, uint16_t len);
void SPI1_RxDMA(void *data, uint16_t len);
int SPI1_IsReady(void);
void SPI1_Tx(uint8_t data);
uint16_t SPI1_Rx(uint8_t data);
void SPI1_SetCallBack(void (*TX_READY)(void),
                      void (*RX_READY)(void),
                      void (*DMATX_READY)(void),
                      void (*DMARX_READY)(void));
*/

#endif
