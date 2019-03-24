#include <stdint.h>
#include "stm32f1xx.h"
#include "fsm_rtc.h"

static uint8_t state, _state;


static int RTC_Timer_Init(void)
{
  __IO int StartUpCounter;
  
  RCC->APB1ENR |= (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN);
  PWR->CR |= PWR_CR_DBP;
  
  //Если RTC не инициализированы
  if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)
  {
    //сбрасываем Backup домен
    RCC->BDCR |= RCC_BDCR_BDRST;
    RCC->BDCR &= ~RCC_BDCR_BDRST;
    
    //включаем генератор 32768Гц
    RCC->BDCR |= RCC_BDCR_LSEON;
    
    //Ждем успешного запуска или окончания тайм-аута
    for(StartUpCounter=0; ; StartUpCounter++)
    {
      //Если успешно запустилось, то 
      //выходим из цикла
      if(RCC->BDCR & RCC_BDCR_LSERDY)
        break;
      
      //Если не запустилось, то
      //отключаем все, что включили
      //и возвращаем ошибку
      if(StartUpCounter > 216000000)
      {
        RCC->BDCR &= ~RCC_BDCR_LSEON; //Останавливаем LSE
        return 2;
      }
    }
    
    //Выбор источника тактирования RTC
    //генератора LSE
    RCC->BDCR |= (0x01 << RCC_BDCR_RTCSEL_Pos);
    
    //Включаем тактирование RTC
    RCC->BDCR |= RCC_BDCR_RTCEN;
  }
  
  
  //Синхронизация
  RTC->CRL &= ~RTC_CRL_RSF;
  while(!(RTC->CRL & RTC_CRL_RSF))
    ;
  
  /*
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  RTC->CRH |= RTC_CRH_SECIE;
  */
  
  //NVIC_EnableIRQ(RTC_IRQn);
  
  return 0;
}


void InitRTC(void)
{
  RTC_Timer_Init();
}

void ProcessRTC(void)
{
  
}

