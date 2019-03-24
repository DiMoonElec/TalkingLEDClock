#include <stdint.h>
#include "stm32f1xx.h"
#include "loader.h"
#include "crc16.h"
#include "uart.h"
#include "at45db321.h"
#include "spi.h"


#if 0

/*
  Протокол обмена - текстовый HEX
  Начало команды: '\n'
  Конец команды:  '\r'
  Формат команды
  "\nCCLLLLLLLLDDDDDDDD...DDRRRR\r"
  CC - код команды 
  LLLLLLLL - количество байт данных
  DD... - полезные данные
  RRRR - контрольная сумма
  Контрольная сумма стичается от 
   кода команды, количества байт данных и самих данных
   т.е. от этих данных: "CCLLLLLLLLDDDDDDDD...DD"

  Формат ответа следующий:
  "\nCCLLLLLLLLDDDDDDDD...DDRRRR\r"
  CC - ответ (RESP_OK, RESP_CMD_ERR, RESP_CRC_ERR)
  LLLLLLLL - количество полезных байт ответа
  RRRR - контрольная сумма
*/


#define CMD_WRITE_PAGE          0x13
#define CMD_READ_PAGE           0x15
#define CMD_GET_PAGE_SIZE       0x42

#define RESP_OK                 0x01
#define RESP_CMD_ERR            0x04
#define RESP_CRC_ERR            0x13

#define PAGE_SIZE               528

#define UART_READ_CHAR()    uart_getc()   
#define UART_WRITE_CHAR(x)  uart_putc((x))

static uint8_t data_buff[PAGE_SIZE];

static void Data2HEX(uint8_t data, uint8_t *HEXStr)
{
  uint8_t tmp;
  tmp = (data >> 4) & 0x0F;
  if(tmp <= 9)
    HEXStr[0] = tmp + '0';
  else
    HEXStr[0] = (tmp - 10)  + 'A';
  
  
  tmp = data & 0x0F;
  if(tmp <= 9)
    HEXStr[1] = tmp + '0';
  else
    HEXStr[1] = (tmp - 10)  + 'A';
}

static int IsHEX(uint8_t *HEXStr)
{
  if(!((HEXStr[0] >= '0' && HEXStr[0] <= '9')
     || HEXStr[0] >= 'A' && HEXStr[0] <= 'F'))
    return 0;
  
  if(!((HEXStr[1] >= '0' && HEXStr[1] <= '9')
     || HEXStr[1] >= 'A' && HEXStr[1] <= 'F'))
    return 0;
  
  return 1;
}

static int16_t HEX2Data(uint8_t *HEXStr)
{
  uint8_t tmp = 0;
  if(!IsHEX(HEXStr))
    return -1;
  

  if(HEXStr[0] >= '0' && HEXStr[0] <= '9')
    tmp |= HEXStr[0] - '0';
  else
    tmp |= (HEXStr[0] - 'A') + 10;
  
  tmp <<= 4;
  
  if(HEXStr[1] >= '0' && HEXStr[1] <= '9')
    tmp |= HEXStr[1] - '0';
  else
    tmp |= (HEXStr[1] - 'A') + 10;
  
  return tmp;
}

static void SendString(uint8_t *str, int len)
{
  for(int i=0; i<len; i++)
    UART_WRITE_CHAR(str[i]);
}

static void Pack(uint8_t resp_code, uint32_t data_len, void *data, uint16_t crc)
{
  uint8_t tmpstr[2];
  uint8_t tmp;
  
  //Начало ответа
  UART_WRITE_CHAR('\n');
  
  //Отправка кода ответа
  Data2HEX(resp_code, tmpstr);
  SendString(tmpstr, 2);
  
  
  //Отправка количества данных
  tmp = (uint8_t)(data_len >> 24);
  Data2HEX(tmp, tmpstr);
  SendString(tmpstr, 2);
  
  tmp = (uint8_t)(data_len >> 16);
  Data2HEX(tmp, tmpstr);
  SendString(tmpstr, 2);
  
  tmp = (uint8_t)(data_len >> 8);
  Data2HEX(tmp, tmpstr);
  SendString(tmpstr, 2);
  
  tmp = (uint8_t)(data_len);
  Data2HEX(tmp, tmpstr);
  SendString(tmpstr, 2);
  
  //Отправка полезных данных
  if(data_len != 0)
  {
    for(int i=0; i<data_len; i++)
    {
      tmp = *(((uint8_t *)(data)) + i);
      Data2HEX(tmp, tmpstr);
      SendString(tmpstr, 2);
    }
  }
  
  //Отправка CRC
  tmp = (uint8_t)(crc >> 8);
  Data2HEX(tmp, tmpstr);
  SendString(tmpstr, 2);
  
  tmp = (uint8_t)(crc);
  Data2HEX(tmp, tmpstr);
  SendString(tmpstr, 2);
  
  //Конец ответа
  UART_WRITE_CHAR('\r');
}

static void Resp(uint8_t resp_code, uint32_t data_len, void *data)
{
  Crc16Reset();
  Crc16(resp_code);
  Crc16((uint8_t)(data_len >> 24));
  Crc16((uint8_t)(data_len >> 16));
  Crc16((uint8_t)(data_len >> 8));
  Crc16((uint8_t)(data_len));
  
  for(int i=0; i<data_len; i++)
    Crc16(*(((uint8_t *)data) + i));
  
  Pack(resp_code, data_len, data, Crc16Get());
}

static int Accept(uint8_t *cmd, uint32_t *data_len, void *data_buff, uint32_t max_len)
{
  uint8_t c;
  
  for(;;)
  {
    /*
    c = UART_READ_CHAR();
    if(
    */
  }
}


void loader_run(void)
{
  uart_init();
  /*
  static uint8_t tmp[2];
  int i;
  for(i=0; i<255; i++)
  {
    Data2HEX(i, tmp);
    if(HEX2Data(tmp) != i)
      asm("nop");
  }
  */
  
  static uint8_t tmp[] = {4,8};
  
  Resp(0x13, 2, tmp);
  //Pack(0x13, 2, tmp, 0xff);
  
  for(;;)
  {
    
  }
}

#endif


#define PAGE_SIZE   528


#define FLASH_CS_PORT   PORTB
#define FLASH_CS_DDR    DDRB
#define FLASH_CS_PIN    3

void FLASH_CS_INIT(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; //Включаем тактирование порта PB
  
  //настройка PB0 на выход
  GPIOB->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
  GPIOB->CRL |= (3<<GPIO_CRL_MODE0_Pos);
  
}
#define FLASH_CS_SELECT()     GPIOB->BRR = (1<<0)
#define FLASH_CS_UNSELECT()   GPIOB->BSRR = (1<<0)



#define CMD_SELECT_CHIP 		1
#define CMD_UNSELECT_CHIP 		2

#define CMD_TRANSFER 			3

#define CMD_SET_PAGE                    12
#define CMD_WRITE_PAGE                  13
#define CMD_TRANSMIT_PAGE               14
#define CMD_SET0_PAGE                   15
#define CMD_READ_PAGE                   20



#define OP_COMPLITE			1
#define OP_ERROR                        2
#define TRANSFER_ERR                    3




static uint8_t data_buff[PAGE_SIZE];

#define UART_READ_CHAR()    uart_getc()   
#define UART_WRITE_CHAR(x)  uart_putc((x))


#define SPI_TRANSFER(x)     SPI1_Transfer((x))
#define SPI_SELECT()        FLASH_CS_SELECT()
#define SPI_UNSELECT()      FLASH_CS_UNSELECT()



void loader_run(void)
{
  uint32_t Page = 0;
  uart_init();
  at45_init();
  /*
  at45_init();
  
  uart_init();
  uart_enable();
  */
  /*
  DDRC = 0xFF;
  PORTC = 0x00;
  */
  uint16_t i;
  char c;
  unsigned short crc_data;
  
  UART_WRITE_CHAR('1');
  
  for(;;)
  {
    c = UART_READ_CHAR();
    
    switch(c)
    {
    case CMD_SET_PAGE:
      Page = 0;
      c = UART_READ_CHAR();
      Page |= c;
      
      c = UART_READ_CHAR();
      Page |= (c<<8);
      
      c = UART_READ_CHAR();
      Page |= (c<<16);
      
      c = UART_READ_CHAR();
      Page |= (c<<24);
      
      UART_WRITE_CHAR(OP_COMPLITE);
      
      break;
      /**************************************/
    case CMD_TRANSMIT_PAGE:
      for(i=0; i<PAGE_SIZE; i++)
        data_buff[i] = UART_READ_CHAR();
      c = UART_READ_CHAR();
      crc_data = UART_READ_CHAR() << 8 | c;
      
      if(crc_data == Crc16(data_buff, PAGE_SIZE))
        UART_WRITE_CHAR(OP_COMPLITE);
      else
        UART_WRITE_CHAR(TRANSFER_ERR);
      break;
      /**************************************/
    case CMD_WRITE_PAGE:
      for(i=0;i<8;i++)
      {
        while(!at45_IsReady()) ;
        at45_BuffWrite(1, data_buff);
      
        while(!at45_IsReady()) ;
        /*
        at45_BuffRead(1, check_buff);
        */
        //if(BuffCompl(data_buff, check_buff, PAGE_SIZE))
        {
           at45_BuffToMainMem(1,Page);
           Page++;
           UART_WRITE_CHAR(OP_COMPLITE);
           break;
        }
      }
      
      UART_WRITE_CHAR(TRANSFER_ERR);
      
      break;
      /**************************************/
    case CMD_SET0_PAGE:
      Page = 0;
      UART_WRITE_CHAR(OP_COMPLITE);
      break;
      /**************************************/
    case CMD_READ_PAGE:
      c = UART_READ_CHAR();
      at45_MainMemToBuff(1, (c<<8) | UART_READ_CHAR());
      while(!at45_IsReady()) ;
      at45_BuffRead(1, data_buff);
      
      for(i=0;i<PAGE_SIZE;i++)
        UART_WRITE_CHAR(data_buff[i]);
      crc_data = Crc16(data_buff, PAGE_SIZE);
      UART_WRITE_CHAR(crc_data>>8);
      UART_WRITE_CHAR(crc_data);
      break;
      /**************************************/
      
    case CMD_SELECT_CHIP:
      SPI_SELECT();
      UART_WRITE_CHAR(OP_COMPLITE);
      break;
      /**************************************/
			
    case CMD_UNSELECT_CHIP:
      SPI_UNSELECT();
      UART_WRITE_CHAR(OP_COMPLITE);
      break;
      /**************************************/
      
    case CMD_TRANSFER:
      UART_WRITE_CHAR(SPI_TRANSFER(UART_READ_CHAR()));
      break;
      /**************************************/
      
    default:
      UART_WRITE_CHAR(OP_ERROR);
      break;
    }
  }
  
  
}

