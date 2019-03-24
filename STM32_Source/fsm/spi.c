#include <stdint.h>
#include "stm32f1xx.h"
#include "spi.h"

//Инициализация SPI1
void SPI_Init(void)
{
  //Включаем тактирование SPI1 и GPIOA
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN | RCC_APB2ENR_IOPAEN;
  
  /**********************************************************/
  /*** Настройка выводов GPIOA на работу совместно с SPI1 ***/
  /**********************************************************/
  
  //PA7 - MOSI
  //PA6 - MISO
  //PA5 - SCK
  //Для начала сбрасываем все конфигурационные биты в нули
  GPIOA->CRL &= ~(GPIO_CRL_CNF5_Msk | GPIO_CRL_MODE5_Msk 
                | GPIO_CRL_CNF6_Msk | GPIO_CRL_MODE6_Msk
                | GPIO_CRL_CNF7_Msk | GPIO_CRL_MODE7_Msk);
  
  //Настроаиваем
  //SCK: MODE5 = 0x03 (11b); CNF5 = 0x02 (10b)
  GPIOA->CRL |= (0x02<<GPIO_CRL_CNF5_Pos) | (0x03<<GPIO_CRL_MODE5_Pos);
  
  //MISO: MODE6 = 0x00 (00b); CNF6 = 0x01 (01b)
  GPIOA->CRL |= (0x01<<GPIO_CRL_CNF6_Pos) | (0x00<<GPIO_CRL_MODE6_Pos);
  
  //MOSI: MODE7 = 0x03 (11b); CNF7 = 0x02 (10b)
  GPIOA->CRL |= (0x02<<GPIO_CRL_CNF7_Pos) | (0x03<<GPIO_CRL_MODE7_Pos);
  
  /*
  //SS MODE4 = 0x03 (11b); CNF4 = 0x02 (10b)
  GPIOA->CRL |= (0x02<<GPIO_CRL_CNF4_Pos) | (0x03<<GPIO_CRL_MODE4_Pos);
  */
  
  /**********************/
  /*** Настройка SPI1 ***/
  /**********************/
  
  SPI1->CR1 = 0<<SPI_CR1_DFF_Pos  //Размер кадра 8 бит
    | 0<<SPI_CR1_LSBFIRST_Pos     //MSB first
    | 1<<SPI_CR1_SSM_Pos          //Программное управление SS
    | 1<<SPI_CR1_SSI_Pos          //SS в высоком состоянии
    | 0x04<<SPI_CR1_BR_Pos        //Скорость передачи: F_PCLK/32
    | 1<<SPI_CR1_MSTR_Pos         //Режим Master (ведущий)
    | 0<<SPI_CR1_CPOL_Pos | 0<<SPI_CR1_CPHA_Pos; //Режим работы SPI: 0
  
  NVIC_EnableIRQ(SPI1_IRQn); //Разрешаем прерывания от SPI1
  
  SPI1->CR1 |= 1<<SPI_CR1_SPE_Pos; //Включаем SPI
}


int32_t tx_index = 0;
int32_t tx_len = 0;
uint8_t *tx_data;

void SPI1_Tx(uint8_t *data, int32_t len)
{
  if(len<=0)
    return;
  
  //Ждем, пока SPI освободится от предыдущей передачи
  while(SPI1->SR & SPI_SR_BSY)
    ;
  
  
  tx_index = 0;
  tx_len = len;
  tx_data = data;
  
  /*
  SPI1->DR = data[tx_index];
  tx_index++;
  */
  SPI1->CR2 |= (1<<SPI_CR2_TXEIE_Pos); 
}

void SPI1_IRQHandler(void)
{
  SPI1->DR = tx_data[tx_index]; //Записываем новое значение в DR
  tx_index++; //увеличиваем счетчик переданных байт на единицу
  
  //если все передали, то отключаем прерывание
  if(tx_index >= tx_len)
    SPI1->CR2 &= ~(1<<SPI_CR2_TXEIE_Pos); 
}


