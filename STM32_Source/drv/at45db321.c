#include <stdint.h>
#include "stm32f1xx.h"
#include "spi.h"
#include "at45db321.h"


#define SPI_INIT()      SPI1_Init()
#define SPI_READ()      SPI1_Transfer(0x00)
#define SPI_WRITE(x)    SPI1_Transfer((x))

/*
  CS pin: PB0 
*/

#define AT45_SELECT()   GPIOB->BRR = (1<<0)
#define AT45_UNSELECT() GPIOB->BSRR = (1<<0)

static void AT45_INIT(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; //Включаем тактирование порта PB
  
  //настройка PB0 на выход
  GPIOB->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
  GPIOB->CRL |= (3<<GPIO_CRL_MODE0_Pos);
}

#define PAGE_SIZE   528

/*
  Eieoeaeecaoey iee?inoaiu flash-iaiyoe
  e iineaaiaaoaeuiiai ii?oa SPI
*/
void at45_init(void)
{
  SPI_INIT();
  AT45_INIT();  
}

/*
  buff - iiia? aioo?aiiaai aooa?a, 1 eee 2
  *data - aaiiua aey caiene a aooa?, ?acia? 528 aaeo 
*/
void at45_BuffWrite(uint8_t buff, uint8_t *data)
{
  uint16_t i;
  
  AT45_UNSELECT();
  AT45_SELECT();
  
  if(buff == 1)
    SPI_WRITE(0x84);
  else
    SPI_WRITE(0x87);
  
  SPI_WRITE(0x00);
  SPI_WRITE(0x00);
  SPI_WRITE(0x00);
  
  //SPI_WRITE(0x00);
 
  for(i=0; i<PAGE_SIZE; i++)
    SPI_WRITE(data[i]);
  
  AT45_UNSELECT();
}


/*
  buff - iiia? aioo?aiiaai aooa?a, 1 eee 2
  *data - aaiiua, ?acia? 528 aaeo 
*/
void at45_BuffRead(uint8_t buff, uint8_t *data)
{
  uint16_t i;
  
  AT45_UNSELECT();
  AT45_SELECT();
  
  
  if(buff == 1)
    SPI_WRITE(0xD1);
  else
    SPI_WRITE(0xD3);
  
  SPI_WRITE(0x00);
  SPI_WRITE(0x00);
  SPI_WRITE(0x00);
  
  //SPI_WRITE(0x00);
 
  for(i=0; i<PAGE_SIZE; i++)
    data[i] = SPI_READ();
  
  
  AT45_UNSELECT();
}

//void at45_ReadMainMem(void *data, uint16_t BufferStartAddress, uint16_t len)
/*
void at45_BuffReadOffset(uint8_t nbuff, void *data, uint16_t BufferStartAddress, uint16_t len)
{
  uint16_t i;
  
  AT45_UNSELECT();
  AT45_SELECT();
  
  //SPI_WRITE(0xD2);
  
  
  if(nbuff == 1)
    SPI_WRITE(0xD1);
  else
    SPI_WRITE(0xD3);
  
  
  SPI_WRITE(0x00);
  SPI_WRITE((BufferStartAddress & 0x0300)>>8);
  SPI_WRITE(BufferStartAddress & 0xFF);
  
  SPI_WRITE(0x00);
 
  for(i=0; i<len; i++)
    *(((uint8_t *)data) + i) = SPI_READ();
  
  
  AT45_UNSELECT();
}
*/
/*
  ia?anueea aioo?aiiaai aooa?a a iniiaio? flash-iaiyou
  buff - iiia? aooa?a 1 eee 2
  page - iiia? no?aieou a iniiaiie iaiyoe
*/
void at45_BuffToMainMem(uint8_t buff, uint32_t page)
{
  AT45_UNSELECT();
  AT45_SELECT();
  
  if(buff == 1)
    SPI_WRITE(0x83);
  else
    SPI_WRITE(0x86);
  
  page <<= 10;
  
  SPI_WRITE((uint8_t)(page >> 16));
  SPI_WRITE((uint8_t)(page >> 8));
  SPI_WRITE((uint8_t)(page));
  
  AT45_UNSELECT();
}

/*
  Ia?aneaou aaiiua ec iniiaiie iaiyoe ai aioo?aiiee aooa?
  buff - iiia? aooa?a 1 eee 2
  page - no?aieoa a iniiaiie iaiyoe
*/
void at45_MainMemToBuff(uint8_t buff, uint32_t page)
{
  AT45_UNSELECT();
  AT45_SELECT();
  
  if(buff == 1) 
    SPI_WRITE(0x53); 
  else 
    SPI_WRITE(0x55);
  
  page <<= 10;
  
  SPI_WRITE((uint8_t)(page >> 16));
  SPI_WRITE((uint8_t)(page >> 8));
  SPI_WRITE((uint8_t)(page));
  
  AT45_UNSELECT();
}

/*
  I?iaa?ea aioiaiinoe iaiyoe
*/
uint8_t at45_IsReady(void)
{
  AT45_UNSELECT();
  AT45_SELECT();
  
  SPI_WRITE(0xD7);
  
  uint8_t tmp = SPI_READ();
  
  AT45_UNSELECT();
  
  if ((tmp & 0x80) != 0)
    return 1;
  
  return 0;
}

/*
  ?oaiea len aaeo, ia?eiay n aa?ana address  a aooa? *buff
*/
void at45_Read(void *buff, uint32_t address, uint16_t len)
{
  //uint8_t *mem_ptr = (uint8_t *)buff;
  
  at45_StreamStart(address);
  
  uint16_t i;
  
  for(i=0; i<len; i++)
  {
    *(((uint8_t *)buff) + i) = at45_StreamRead();
  }
  
  at45_StreamStop();
}

/*
  Eieoeaeecaoey iioieiaiai ?oaiey n flash-iaiyoe
*/

void at45_StreamStart(uint32_t address)
{
  AT45_UNSELECT();
  AT45_SELECT();
  
  SPI_WRITE(0x03);
  
  uint16_t page = address / 528;
  uint16_t b_adr = address % 528;
  
  uint8_t b1 = (uint8_t)(page >> 6);
  uint8_t b2 = (uint8_t)((page << 2) | (b_adr >> 8));
  uint8_t b3 = (uint8_t)(b_adr & 0xFF);
 
  SPI_WRITE(b1);
  SPI_WRITE(b2);
  SPI_WRITE(b3);
}

/*
  Caaa?oaiea iioieiaiai ?oaiey
*/
void at45_StreamStop(void)
{
  AT45_UNSELECT();
}

/*
  ?oaiea aaeoa aaiiuo ec flash-iaiyoe 
  a iioieiaii ?a?eia
*/

uint8_t at45_StreamRead(void)
{
  return SPI_READ();
  /*
  if(StreamPageOffset >= 528)
  {
    StreamPageOffset = 0;
    StreamPage++;
    _stream_start(StreamPage, StreamPageOffset);
  }
  StreamPageOffset++;
  return SPI_READ();
  */
}

/*
void at45_StreamDMARead(void *buff, uint16_t len, void (*CallBack)(void))
{
  SPI1_DMARead(buff, len, CallBack);
}
*/













#if 0

#include "stm32f1xx.h"
#include "spi.h"
#include "at45db321.h"

/*
  CS pin: PB0 
*/

static uint8_t tmp[16];
static int tmplen;
static void *tmpref;

#define AT45_SELECT()   GPIOB->BRR = (1<<0)
#define AT45_UNSELECT() GPIOB->BSRR = (1<<0)

static uint8_t state;
static char IsRdy;

enum
{
  STATE_READ = 0,
  STATE_READ_1,
  STATE_1
};



static void txrdy_Handler(void)
{
  if(state == STATE_READ)
  {
    state = STATE_READ_1;
    //SPI1_SetCallBack(0, 0, txrdy_Handler, 0);
    SPI1_TxDMA(tmpref, tmplen);
  }
  else if(state == STATE_READ_1)
  {
    AT45_UNSELECT();  
    SPI1_SetCallBack(0, 0, 0, 0);
    IsRdy = 1;
  }
}



void at45_init(void)
{
  IsRdy = 1;
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; //Включаем тактирование порта PB
  
  //настройка PB0 на выход
  GPIOB->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
  GPIOB->CRL |= (1<<GPIO_CRL_MODE0_Pos);
  AT45_UNSELECT();
}

char at45_isrdy(void)
{
  return IsRdy;
}

void at45_Read(void *buff, uint32_t address, uint16_t len)
{
  IsRdy = 0;
  uint8_t index = 0;
  
  tmpref = buff;
  tmplen = len;
  
  AT45_UNSELECT();
  AT45_SELECT();
  
  tmp[index] = 0x03;
  index++;
  
  uint16_t page = address / 528;
  uint16_t b_adr = address % 528;
  
  uint8_t b1 = (uint8_t)(page >> 6);
  uint8_t b2 = (uint8_t)((page << 2) | (b_adr >> 8));
  uint8_t b3 = (uint8_t)(b_adr & 0xFF);
 
  tmp[index] = b1;
  index++;
  
  tmp[index] = b2;
  index++;
  
  tmp[index] = b3;
  index++;
  
  tmp[index] = 0x00;
  index++;
  
  SPI1_SetCallBack(0, 0, 0, txrdy_Handler);
  SPI1_TxDMA(tmp, index-1);
  
  state = STATE_READ;
}


#endif
