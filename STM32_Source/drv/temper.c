#include <stdint.h>
#include "stm32f1xx.h"
#include "temper.h"
#include "timers.h" //FSM Timers

#define DS18B20_SKIP_ROM         0xcc
#define DS18B20_CONVERT_T        0x44
#define DS18B20_READ_SCRATCHPAD  0xbe


/*
  PA9 - indoor sensor
*/


/*****************************************************/

static uint8_t state, _state;

enum
{
  STATE_MAIN = 0,
  STATE_INDOOR_UPDATE,
  STATE_INDOOR_UPDATE_1,
  STATE_OUTDOOR_UPDATE,
  STATE_OUTDOOR_UPDATE_1,
  STATE_DELAY
};

static float temper_indoor;
static uint8_t flag_indoor_exist;
static uint8_t flag_indoor_update;

static float temper_outdoor;
static uint8_t flag_outdoor_exist;
static uint8_t flag_outdoor_update;

/*****************************************************/


static unsigned char crc8 = 0;

static unsigned char dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

unsigned char docrc8(unsigned char value)
{
   // See Application Note 27
   
   // TEST BUILD
   crc8 = dscrc_table[crc8 ^ value];
   return crc8;
}

/*****************************************************/

#define UART_BRR_9600           0x1D4C
#define UART_BRR_115200         0x0271

static void uart_enable(void)
{
  USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
  //USART1->CR1 |= USART_CR1_UE;
}

static void uart_disable(void)
{
  USART1->CR1 &= ~(USART_CR1_TE | USART_CR1_RE);
  //USART1->CR1 &= ~USART_CR1_UE;
}

static void uart_baud(uint16_t val)
{
  USART1->BRR = val;
}


/*************************************/
/**
  BUS0 (OWI0)
**/

static void PA9_AltFunc(void)
{
  uint32_t tmp = GPIOA->CRH;
  
  tmp &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
  tmp |= (0x02 << GPIO_CRH_MODE9_Pos | 0x03 << GPIO_CRH_CNF9_Pos);
  GPIOA->CRH = tmp;
}

static void PA9_Input(void)
{
  uint32_t tmp = GPIOA->CRH;
  
  tmp &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
  tmp |= (0x00 << GPIO_CRH_MODE9_Pos | 0x01 << GPIO_CRH_CNF9_Pos);
  GPIOA->CRH = tmp;
}

/**
  BUS1 (OWI1)
**/
static void PB6_AltFunc(void)
{
  uint32_t tmp = GPIOB->CRL;
  
  tmp &= ~(GPIO_CRL_CNF6 | GPIO_CRL_MODE6);
  tmp |= (0x02 << GPIO_CRL_MODE6_Pos | 0x03 << GPIO_CRL_CNF6_Pos);
  GPIOB->CRL = tmp;
}

static void PB6_Input(void)
{
  uint32_t tmp = GPIOB->CRL;
  
  tmp &= ~(GPIO_CRL_CNF6 | GPIO_CRL_MODE6);
  tmp |= (0x00 << GPIO_CRL_MODE6_Pos | 0x01 << GPIO_CRL_CNF6_Pos);
  GPIOB->CRL = tmp;
}

/*************************************/
static void uart_reinit(void)
{
  //—брос UART
  RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
  RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
  
  USART1->CR1 = USART_CR1_UE;
  USART1->CR2 = 0;
  USART1->CR3 = USART_CR3_HDSEL; //Half-duplex selection
}

static void OWIBus_Select(uint8_t bus)
{
  if(bus) //bus == 1
  {
    PA9_Input();
    AFIO->MAPR |= (AFIO_MAPR_USART1_REMAP);
    PB6_AltFunc();
    uart_reinit();
  }
  else //bus == 0
  {
    PB6_Input();
    AFIO->MAPR &= ~(AFIO_MAPR_USART1_REMAP);
    PA9_AltFunc();
    uart_reinit();
  }
}

/*************************************/



static void uart_init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //тактирование AFIO
  
  //GPIO init alternate function open drain
  PA9_Input();
  PB6_Input();
  /*
  GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
  GPIOA->CRH |= (0x02 << GPIO_CRH_MODE9_Pos | 0x03 << GPIO_CRH_CNF9_Pos);
  
  GPIOB->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_MODE6);
  GPIOB->CRL |= (0x02 << GPIO_CRL_MODE6_Pos | 0x03 << GPIO_CRL_CNF6_Pos);
  */
  
  //USART1 init
  /*
  USART1->CR1 = USART_CR1_UE;
  USART1->CR2 = 0;
  USART1->CR3 = USART_CR3_HDSEL; //Half-duplex selection
  */
  /*
  USART1->CR1 = USART_CR1_TE //Transmitter enable
    | USART_CR1_RE; //Receiver enable
  */
  
  //uart_baud(UART_BRR_115200);
  //OWIBus_Select(0);
  //uart_enable();
}

static void uart_tx(uint8_t val)
{
  USART1->DR = val;
}

static uint8_t uart_rx(void)
{
  return USART1->DR;
}

static int uart_isrdy(void)
{
  if(USART1->SR & USART_SR_TC)
    return 1;
  return 0;
}

static void uart_clrrdy(void)
{
  USART1->SR &= ~USART_SR_TC;
}

static void uart_claer_rxne(void)
{
  USART1->SR &= ~USART_SR_RXNE;
}

static int uart_get_rxne(void)
{
  if(USART1->SR & USART_SR_RXNE)
    return 1;
  return 0;
}

/************************************************************/
static uint8_t OW_Reset(void) 
{
  uint8_t ow_presence;
  
  uart_baud(UART_BRR_9600);
  
  uart_clrrdy();
  uart_tx(0xF0);
  while(!uart_isrdy())
    ;
  
  ow_presence = uart_rx();
  uart_baud(UART_BRR_115200);
  
  if(ow_presence != 0xF0)
    return 1;
  
  return 0;
}

static uint8_t readBit(void)
{
  uint8_t tmp;
  uart_claer_rxne();
  uart_tx(0xFF);
    
  while(!uart_get_rxne())
    ;
  
  tmp = uart_rx();
  
  return (tmp == 0xFF ? 1: 0);
}


static char OW_SendByte(char data)
{
  int i;
  
  for(i=0; i<8; i++)
  {
    uart_claer_rxne();
    
    if(data & 0x01)
    {
      uart_tx(0xFF);
    }
    else
    {
      uart_tx(0x00);
    }
    
    data >>= 1;
    
    
    while(!uart_get_rxne())
    ;
    
    //USART_ReceiveData(USART2);
  }
  
  return 0;
}

/************************************************************/




static char ds18b20_convert(void)
{
  char c;
  c = OW_Reset();
  
  if(!c)
    return 0;
  
  OW_SendByte(DS18B20_SKIP_ROM);
  OW_SendByte(DS18B20_CONVERT_T);
  
  return 1;
}

static int ds18b20_tread(float *temper)
{
  char c;
  c = OW_Reset();
  
  if(!c)
    return 0;
  
  OW_SendByte(DS18B20_SKIP_ROM);
  OW_SendByte(DS18B20_READ_SCRATCHPAD);
  
  //return readTemperature();
  
  char data[9];
  for(int k=0; k<9; k++)
  {
    data[k] = 0;
    for (uint8_t i = 0; i < 8; i++)
      data[k] += readBit() << i;
  }
  
  crc8 = 0;
  for(int i=0; i<9; i++)
    docrc8(data[i]);
  
  if(crc8 != 0)
    return 0;
      
  *temper = (float)((data[0] | data[1] << 8)) / 16.0;
  return 1;
}

/************************************************************/
void InitTemper(void)
{
  uart_init();
  state = STATE_MAIN;
  _state = state + 1;
  
  flag_indoor_update = 0;
  flag_indoor_exist = 0;
  
  flag_outdoor_update = 0;
  flag_outdoor_exist = 0;
}


void ProcessTemper(void)
{
  uint8_t entry;
  
  if(_state != state)
    entry = 1;
  else 
    entry = 0;
  
  _state = state;
  
  
  switch(state)
  {
  case STATE_MAIN:
    state = STATE_INDOOR_UPDATE;
    break;
    /***************************************************/
    
  case STATE_INDOOR_UPDATE:
    if(entry)
    {
      uart_disable();
      OWIBus_Select(0);
      uart_enable();
      //flag_indoor_exist = ds18b20_convert();
      ResetTimer(TIMER_TEMPER_GENERAL);
    }
    
    if(GetTimer(TIMER_TEMPER_GENERAL) >= TMR_SEC/2)
    {
      state = STATE_INDOOR_UPDATE_1;
    }
    
    break;
    /***************************************************/
    
  case STATE_INDOOR_UPDATE_1:
    if(entry)
    {
      flag_indoor_exist = ds18b20_convert();
      ResetTimer(TIMER_TEMPER_GENERAL);
    }
    
    if(flag_indoor_exist)
    {
      if(GetTimer(TIMER_TEMPER_GENERAL) >= TMR_SEC * 2)
      {
        flag_indoor_exist = ds18b20_tread(&temper_indoor);
        flag_indoor_update = 1;
        state = STATE_OUTDOOR_UPDATE;
      }
    }
    else
    {
      state = STATE_OUTDOOR_UPDATE;
    }
    
    break;
    /***************************************************/
    
  case STATE_OUTDOOR_UPDATE:
    if(entry)
    {
      uart_disable();
      OWIBus_Select(1);
      uart_enable();
      //flag_outdoor_exist = ds18b20_convert();
      ResetTimer(TIMER_TEMPER_GENERAL);
    }
    
    if(GetTimer(TIMER_TEMPER_GENERAL) >= TMR_SEC/2)
    {
      state = STATE_OUTDOOR_UPDATE_1;
    }
    break;
    /***************************************************/
    
  case STATE_OUTDOOR_UPDATE_1:
    if(entry)
    {
      flag_outdoor_exist = ds18b20_convert();
      ResetTimer(TIMER_TEMPER_GENERAL);
    }
    
    if(flag_outdoor_exist)
    {
      if(GetTimer(TIMER_TEMPER_GENERAL) >= TMR_SEC * 2)
      {
        flag_outdoor_exist = ds18b20_tread(&temper_outdoor);
        flag_outdoor_update = 1;
        state = STATE_DELAY;
      }
    }
    else
    {
      state = STATE_DELAY;
    }
    break;
    /***************************************************/
    
  case STATE_DELAY:
    if(entry)
    {
      ResetTimer(TIMER_TEMPER_GENERAL);
    }
    
    if(GetTimer(TIMER_TEMPER_GENERAL) >= TMR_SEC * 30)
      state = STATE_MAIN;
    break;
    /***************************************************/
    
  default:
    state = STATE_MAIN;
  }
}


float GetTemper(int n)
{
  if(n == 0)
    return temper_indoor;
  return temper_outdoor;
}

int GetTemperStatus(int n)
{
  if(n == 0)
  {
    if(flag_indoor_exist == 0)
      return -1;
    
    if(flag_indoor_update == 1)
    {
      flag_indoor_update = 0;
      return 1;
    }
    
    return 0;
  }
  else
  {
    if(flag_outdoor_exist == 0)
      return -1;
    
    if(flag_outdoor_update == 1)
    {
      flag_outdoor_update = 0;
      return 1;
    }
    
    return 0;
  }
}




