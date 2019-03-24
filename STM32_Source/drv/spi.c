#include <stdint.h>
#include "stm32f1xx.h"
#include "spi.h"

void (*DMARxRdy_CallBack)(void) = 0;

//Инициализация SPI1
void SPI1_Init(void)
{
  //Включаем тактирование SPI1 и GPIOA
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN | RCC_APB2ENR_IOPAEN;
  RCC->AHBENR |= RCC_AHBENR_DMA1EN; //Включаем тактирование DMA1
  
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
    | 0x01<<SPI_CR1_BR_Pos        //Скорость передачи: F_PCLK/4
    | 1<<SPI_CR1_MSTR_Pos         //Режим Master (ведущий)
    | 0<<SPI_CR1_CPOL_Pos | 0<<SPI_CR1_CPHA_Pos; //Режим работы SPI: 0
  
  SPI1->CR2 |= 1<<SPI_CR2_TXDMAEN_Pos;
  SPI1->CR2 |= 1<<SPI_CR2_RXDMAEN_Pos;
  
  DMA1->IFCR = DMA_IFCR_CTCIF2 | DMA_IFCR_CGIF2; //Очищаем флаги прерываний
  NVIC_EnableIRQ(DMA1_Channel2_IRQn); //Разрешаем прерывания от 2-го
  //канала DMA
  
  SPI1->CR1 |= 1<<SPI_CR1_SPE_Pos; //Включаем SPI
}


uint16_t SPI1_Transfer(uint16_t data)
{
  SPI1->DR;
  
  SPI1->DR = data;
  while(!(SPI1->SR & SPI_SR_RXNE))
    ;
  
  return SPI1->DR;
}


void SPI1_DMARead(void *data, uint16_t len, void (*CallBack)(void))
{
  static uint8_t _filler = 0xFF;
  
  DMARxRdy_CallBack = CallBack;
  //отключаем канал DMA после предыдущей передачи данных
  DMA1_Channel2->CCR &= ~(1 << DMA_CCR_EN_Pos); 
  
  DMA1_Channel2->CPAR = (uint32_t)(&SPI1->DR); //заносим адрес регистра DR в CPAR
  DMA1_Channel2->CMAR = (uint32_t)data; //заносим адрес данных в регистр CMAR
  DMA1_Channel2->CNDTR = len; //количество передаваемых данных
  
  //Настройка канала DMA
  DMA1_Channel2->CCR = 0 << DMA_CCR_MEM2MEM_Pos //режим MEM2MEM отключен
    | 0x00 << DMA_CCR_PL_Pos //приоритет низкий
    | 0x00 << DMA_CCR_MSIZE_Pos //разрядность данных в памяти 8 бит
    | 0x01 << DMA_CCR_PSIZE_Pos //разрядность регистра данных 16 бит 
    | 1 << DMA_CCR_MINC_Pos //Включить инкремент адреса памяти
    | 0 << DMA_CCR_PINC_Pos //Инкремент адреса периферии отключен
    | 0 << DMA_CCR_CIRC_Pos //кольцевой режим отключен
    | 0 << DMA_CCR_DIR_Pos  //0 - из периферии в память
    | 1 << DMA_CCR_TCIE_Pos; //Прерывание при окончании приема
  DMA1->IFCR = DMA_IFCR_CTCIF2 | DMA_IFCR_CGIF2; //Очищаем флаги прерываний
  
  DMA1_Channel2->CCR |= 1 << DMA_CCR_EN_Pos; //включаем прием данных
  
  
  //////////////////////////////////////////////////////////////////////////////
  
  //отключаем канал DMA после предыдущей передачи данных
  DMA1_Channel3->CCR &= ~(1 << DMA_CCR_EN_Pos); 
  
  DMA1_Channel3->CPAR = (uint32_t)(&SPI1->DR); //заносим адрес регистра DR в CPAR
  DMA1_Channel3->CMAR = (uint32_t)(&_filler); //заносим адрес данных в регистр CMAR
  DMA1_Channel3->CNDTR = len; //количество передаваемых данных
  
  //Настройка канала DMA
  DMA1_Channel3->CCR = 0 << DMA_CCR_MEM2MEM_Pos //режим MEM2MEM отключен
    | 0x00 << DMA_CCR_PL_Pos //приоритет низкий
    | 0x00 << DMA_CCR_MSIZE_Pos //разрядность данных в памяти 8 бит
    | 0x01 << DMA_CCR_PSIZE_Pos //разрядность регистра данных 16 бит 
    | 0 << DMA_CCR_MINC_Pos //Инкремент адреса памяти отключен
    | 0 << DMA_CCR_PINC_Pos //Инкремент адреса периферии отключен
    | 0 << DMA_CCR_CIRC_Pos //кольцевой режим отключен
    | 1 << DMA_CCR_DIR_Pos;  //1 - из памяти в периферию
  
  DMA1_Channel3->CCR |= 1 << DMA_CCR_EN_Pos; //Запускаем процесс
}


void DMA1_Channel2_IRQHandler(void)
{
  DMA1->IFCR = DMA_IFCR_CTCIF2 | DMA_IFCR_CGIF2; //Очищаем флаги прерываний
  if(DMARxRdy_CallBack != 0)
    DMARxRdy_CallBack();
  
  DMA1_Channel2->CCR &= ~(1 << DMA_CCR_EN_Pos); //отключаем канал DMA
}








#if 0


/*
  DMA1: 
    SPI1_RX - Channel 2
    SPI1_TX - Channel 3

*/


#include <stdint.h>
#include "stm32f1xx.h"
#include "spi.h"


static void (*tx_rdy)(void);
static void (*rx_rdy)(void);
static void (*dmarx_rdy)(void);
static void (*dmatx_rdy)(void);

/*
static int32_t tx_index = 0;
static int32_t tx_len = 0;
static uint8_t *tx_data = 0;
static uint8_t *rx_data = 0;

static uint8_t tx_char = 0xFF;

static uint8_t spi_mode;
*/

/*
enum
{
  SPI_MODE_TX = 0,
  SPI_MODE_RX,
  SPI_MODE_RXTX
};
*/

static char flag_dmatx_int = 0;
static char flag_dmarx_int = 0;


#define SPI_INT_TXEIE()         SPI1->CR2 |= (1<<SPI_CR2_TXEIE_Pos)
#define SPI_INT_RXNEIE()        SPI1->CR2 |= (1<<SPI_CR2_RXNEIE_Pos)
#define SPI_INT_DISABLE()       SPI1->CR2 &= (~(1<<SPI_CR2_TXEIE_Pos | 1<<SPI_CR2_RXNEIE_Pos))
#define SPI_TXEIE_DISABLE()     SPI1->CR2 &= (~(1<<SPI_CR2_TXEIE_Pos))
#define SPI_RXNEIE_DISABLE()    SPI1->CR2 &= (~(1<<SPI_CR2_RXNEIE_Pos))


/*
#define DMA_TXRDY_ENABLE()      DMA1_Channel3->CCR |= DMA_CCR_TCIE
#define DMA_RXRDY_ENABLE()      DMA1_Channel2->CCR |= DMA_CCR_TCIE

#define DMA_TXRDY_DISABLE()     DMA1_Channel3->CCR &= ~DMA_CCR_TCIE
#define DMA_RXRDY_DISABLE()     DMA1_Channel2->CCR &= ~DMA_CCR_TCIE
*/

//Инициализация SPI1
void SPI1_Init(void)
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
  
  //NVIC_EnableIRQ(SPI1_IRQn); //Разрешаем прерывания от SPI1
  
  
  SPI1->CR1 |= 1<<SPI_CR1_SPE_Pos; //Включаем SPI
  
  RCC->AHBENR |= RCC_AHBENR_DMA1EN; //Включаем тактирование DMA1
    DMA1->IFCR = DMA_IFCR_CTCIF3
    | DMA_IFCR_CHTIF3 
    | DMA_IFCR_CTEIF3 
    | DMA_IFCR_CGIF3;
  
  DMA1->IFCR = DMA_IFCR_CTCIF2 
    | DMA_IFCR_CHTIF2 
    | DMA_IFCR_CTEIF2 
    | DMA_IFCR_CGIF2;
  
  NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  NVIC_EnableIRQ(DMA1_Channel3_IRQn);
}

void SPI1_TxDMA(void *data, uint16_t len)
{
  static uint8_t _filler = 0;
  
  if(!SPI1_IsReady())
    return;
  
  DMA1->IFCR = DMA_IFCR_CTCIF3
    | DMA_IFCR_CHTIF3 
    | DMA_IFCR_CTEIF3 
    | DMA_IFCR_CGIF3;
  
  SPI1->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
  SPI1->CR2 |= SPI_CR2_RXDMAEN;
  SPI1->CR2 |= SPI_CR2_TXDMAEN;
  
  
  if(flag_dmarx_int)
  {
    DMA1->IFCR = DMA_IFCR_CTCIF2 
    | DMA_IFCR_CHTIF2 
    | DMA_IFCR_CTEIF2 
    | DMA_IFCR_CGIF2;
  
    DMA1_Channel2->CCR &= ~DMA_CCR_EN; //отключаем канал DMA
    
    DMA1_Channel2->CNDTR = len;
    DMA1_Channel2->CPAR = (uint32_t)(&SPI1->DR);
    DMA1_Channel2->CMAR = (uint32_t)(&_filler);
    DMA1_Channel2->CCR = 0x01<<DMA_CCR_PSIZE_Pos;
    DMA1_Channel2->CCR |= DMA_CCR_TCIE;
    DMA1_Channel2->CCR |= DMA_CCR_EN;
  }
  
  DMA1_Channel3->CCR &= ~DMA_CCR_EN; //отключаем канал DMA
  
  DMA1_Channel3->CNDTR = len;
  DMA1_Channel3->CPAR = (uint32_t)(&SPI1->DR);
  DMA1_Channel3->CMAR = (uint32_t)data;
  DMA1_Channel3->CCR = 0x01<<DMA_CCR_PSIZE_Pos
    | DMA_CCR_MINC 
    | DMA_CCR_DIR;

  if(flag_dmatx_int)
    DMA1_Channel3->CCR |= DMA_CCR_TCIE;
  DMA1_Channel3->CCR |= DMA_CCR_EN;  
  
}


void SPI1_RxDMA(void *data, uint16_t len)
{
  static uint8_t _filler = 0xFF;
  
  if(!SPI1_IsReady())
    return;
  
  DMA1->IFCR = DMA_IFCR_CTCIF3
    | DMA_IFCR_CHTIF3 
    | DMA_IFCR_CTEIF3 
    | DMA_IFCR_CGIF3;
  
  DMA1->IFCR = DMA_IFCR_CTCIF2 
    | DMA_IFCR_CHTIF2 
    | DMA_IFCR_CTEIF2 
    | DMA_IFCR_CGIF2;
  
  SPI1->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
  SPI1->CR2 |= SPI_CR2_RXDMAEN;
  SPI1->CR2 |= SPI_CR2_TXDMAEN;
  
  DMA1_Channel2->CCR &= ~DMA_CCR_EN; //отключаем канал DMA
  DMA1_Channel3->CCR &= ~DMA_CCR_EN; //отключаем канал DMA
  
  DMA1_Channel2->CNDTR = len;
  DMA1_Channel2->CPAR = (uint32_t)(&SPI1->DR);
  DMA1_Channel2->CMAR = (uint32_t)data;
  DMA1_Channel2->CCR = 0x01<<DMA_CCR_PSIZE_Pos
    | DMA_CCR_MINC;
  
  if(flag_dmarx_int)
    DMA1_Channel2->CCR |= DMA_CCR_TCIE;
  DMA1_Channel2->CCR |= DMA_CCR_EN;
  
  DMA1_Channel3->CNDTR = len;
  DMA1_Channel3->CPAR = (uint32_t)(&SPI1->DR);
  DMA1_Channel3->CMAR = (uint32_t)(&_filler);
  DMA1_Channel3->CCR = 0x01<<DMA_CCR_PSIZE_Pos
    | DMA_CCR_DIR
    | DMA_CCR_EN;
}

void SPI1_Tx(uint8_t data)
{
  SPI1->DR = data;
}

uint16_t SPI1_Rx(uint8_t data)
{
  return SPI1->DR;
}

int SPI1_IsReady(void)
{
  if((SPI1->SR & SPI_SR_TXE) &&
     (!(SPI1->SR & SPI_SR_BSY)))
  {
    return 1;
  }
  return 0;
}

void SPI1_SetCallBack(void (*TX_READY)(void),
                      void (*RX_READY)(void),
                      void (*DMATX_READY)(void),
                      void (*DMARX_READY)(void))
{
  tx_rdy = TX_READY;
  if(tx_rdy != 0)
    SPI_INT_TXEIE();
  else
    SPI_TXEIE_DISABLE();
  
  rx_rdy = RX_READY;
  if(rx_rdy != 0)
    SPI_INT_RXNEIE();
  else
    SPI_RXNEIE_DISABLE();
  
  
  
  dmatx_rdy = DMATX_READY;
  if(dmatx_rdy != 0)
    flag_dmatx_int = 1;
  else
    flag_dmatx_int = 0;
  
  
    dmarx_rdy = DMARX_READY;
  if(dmarx_rdy != 0)
    flag_dmarx_int = 1;
  else
    flag_dmarx_int = 0;
}

void SPI1_IRQHandler(void)
{
  if(tx_rdy != 0)
    tx_rdy();
  
  if(rx_rdy != 0)
    rx_rdy();
}

void DMA1_Channel2_IRQHandler(void)
{
  DMA1->IFCR = DMA_IFCR_CTCIF2 
    | DMA_IFCR_CHTIF2 
    | DMA_IFCR_CTEIF2 
    | DMA_IFCR_CGIF2;
  
  if(dmarx_rdy != 0)
    dmarx_rdy();
}

void DMA1_Channel3_IRQHandler(void)
{
  DMA1->IFCR = DMA_IFCR_CTCIF3
    | DMA_IFCR_CHTIF3 
    | DMA_IFCR_CTEIF3 
    | DMA_IFCR_CGIF3;
  
  if(dmatx_rdy != 0)
    dmatx_rdy();
}


#endif

