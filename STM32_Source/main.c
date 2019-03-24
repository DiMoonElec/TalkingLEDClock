#include <stdint.h>
#include "stm32f1xx.h"
#include "clock.h"
#include "watch_io.h"
#include "rtc.h"
#include "timers.h"
#include "fsm_main.h"
#include "spi.h"
#include "at45db321.h"
#include "uart.h"
#include "loader.h"
#include "voice.h"
#include "temper.h"
#include "light_sensor.h"

static void flash_mode(void);
static int is_flash_mode(void);

void main(void)
{
  int status = is_flash_mode();
  ClockInit();
  
  //status = 1;
  if(status)
  {
    flash_mode();
  }  
  io_init();
  RTC_Init();
  
  //FSM init
  InitTimers();
  InitFSMMain();
  VoiceInit();
  
  InitTemper();
  
  InitLigthSensor();
  
  for(;;)
  {
    ProcessFSMMain();
    VoiceProcess();
    ProcessTemper();
    ProcessLigthSensor();
    __WFI();
  }
}




#define BUTTON_1_MSK    (1<<3)
#define BUTTON_2_MSK    (1<<4)
#define BUTTON_3_MSK    (1<<5)
#define BUTTON_4_MSK    (1<<8)
#define BUTTON_5_MSK    (1<<7)

static int is_flash_mode(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; //Включаем тактирование GPIOB
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //тактирование AFIO
  
  //Отключаем JTAG, так как он висит 
  //на используемых для кнопок пинах
  AFIO->MAPR |= (0x02<<AFIO_MAPR_SWJ_CFG_Pos); 
  
  //Выводы, к которым подключены кнопки:
  //PB3 - B1
  //PB4 - B2
  //PB5 - B3
  //PB8 - B4
  //PB7 - B5
  
  GPIOB->CRL &= ~(GPIO_CRL_CNF3 | GPIO_CRL_MODE3
                | GPIO_CRL_CNF4 | GPIO_CRL_MODE4
                | GPIO_CRL_CNF5 | GPIO_CRL_MODE5
                | GPIO_CRL_CNF7 | GPIO_CRL_MODE7);
  GPIOB->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_MODE8);
  
  //Настраиваем все выводы на вход с подтяжкой вверх
  GPIOB->CRL |= (0x02<<GPIO_CRL_CNF3_Pos
               | 0x02<<GPIO_CRL_CNF4_Pos
               | 0x02<<GPIO_CRL_CNF5_Pos
               | 0x02<<GPIO_CRL_CNF7_Pos);
  GPIOB->CRH |= (0x02<<GPIO_CRH_CNF8_Pos);
  
  GPIOB->ODR |= (BUTTON_1_MSK 
               | BUTTON_2_MSK 
               | BUTTON_3_MSK 
               | BUTTON_4_MSK 
               | BUTTON_5_MSK);
  
  
  for(int i=0; i<10000; i++)
    asm("nop");
  
  uint16_t tmp = GPIOB->IDR; //сохраняем значение порта
  tmp ^= 0xFFFF;
  
  RCC->APB2RSTR |= (RCC_APB2RSTR_IOPBRST | RCC_APB2RSTR_AFIORST);
  RCC->APB2RSTR &= ~(RCC_APB2RSTR_IOPBRST | RCC_APB2RSTR_AFIORST);
  
  RCC->APB2ENR &= ~RCC_APB2ENR_IOPBEN; //Отключаем тактирование GPIOB
  RCC->APB2ENR &= ~RCC_APB2ENR_AFIOEN; //тактирование AFIO
  
  
  if((tmp & BUTTON_2_MSK) && (tmp & BUTTON_5_MSK)
     && (!(tmp & BUTTON_1_MSK)) && (!(tmp & BUTTON_3_MSK)) && (!(tmp & BUTTON_4_MSK)))
    return 1;
  
  return 0;
  
}

static void flash_mode(void)
{
  //LED_Set(5, 1);
  loader_run();
  
  for(;;)
  {
    ;
  }
  
}