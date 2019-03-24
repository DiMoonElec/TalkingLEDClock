#include <stdint.h>
#include "stm32f1xx.h"
#include "rtc.h"

// (UnixTime = 00:00:00 01.01.1970 = JD0 = 2440588)
//#define JULIAN_DATE_BASE    2440588

static uint32_t rtc_cnt = 0;
static uint8_t rtc_update_flag = 0;

static uint32_t alarm_val = 0;
static uint8_t alarm_IsOn = 0;
static uint8_t alarm_status = 0;

static uint8_t flag_BackupIsInit = 0;






int RTC_Init(void)
{
  __IO int StartUpCounter;
  
  RCC->APB1ENR |= (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN);
  PWR->CR |= PWR_CR_DBP;
  
  flag_BackupIsInit = 1;
  //Если RTC не инициализированы
  if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)
  {
    flag_BackupIsInit = 0;
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
  
  RTC->CRL &= ~RTC_CRL_RSF;
  while(!(RTC->CRL & RTC_CRL_RSF))
    ;
  
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  RTC->CRH |= RTC_CRH_SECIE;
  
  NVIC_EnableIRQ(RTC_IRQn);
  
  return 0;
}


void RTC_GetTime(RTCTime_t* time)
{
  uint32_t RTC_Counter = rtc_cnt;
  uint32_t tmp;
  
  RTC_Counter %= (60*60*24);
  
  tmp = RTC_Counter % 60;
  RTC_Counter /= 60;
  
  time->RTC_Seconds = tmp;
  
  tmp = RTC_Counter % 60;
  RTC_Counter /= 60;
  
  time->RTC_Minutes = tmp;
  
  time->RTC_Hours = RTC_Counter;
}

void RTC_SetTime(RTCTime_t* time)
{
  uint32_t RTC_Counter;
  
  RTC_Counter = time->RTC_Hours*60*60;
  RTC_Counter += time->RTC_Minutes*60;
  RTC_Counter += time->RTC_Seconds;
  
  //Отключам прерывание
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  RTC->CRH &= ~RTC_CRH_SECIE;
  
  rtc_cnt = RTC_Counter;
  
  //Разрешаем запись в регистры
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  RTC->CRL |= RTC_CRL_CNF;
  
  
  //Устанавливаем перувую половину
  //стечного регистра
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  RTC->CNTL = (RTC_Counter & 0xFFFF);
  RTC_Counter >>= 16;
  
  //Устанавливаем вторую половину
  //счетного регистра
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  RTC->CNTH = RTC_Counter;
  
  
  /*
  //Сбрасываем предделитель
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  RTC->DIVH = 0;
  
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  RTC->DIVL = 0;
  */
  
  //Сбрасываем флаг разрешения 
  //записи в регистры
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  RTC->CRL &= ~RTC_CRL_CNF;
  
  //Ждем, пока завершится запись
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  
  
  
  //сбрасываем флаг прерывания
  RTC->CRL &= ~RTC_CRL_SECF; 
  
  
  //Разрешаем прерывание
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  RTC->CRH |= RTC_CRH_SECIE;
  
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
}


int RTC_IsUpdate(void)
{
  if(rtc_update_flag != 0)
  {
    rtc_update_flag = 0;
    return 1;
  }
  else
    return 0;
}

int Alarm_Triggered(void)
{
  if(alarm_status != 0)
  {
    alarm_status = 0;
    return 1;
  }
  else
    return 0;
}

void AlarmSet(RTCTime_t* time)
{
  alarm_val = (time->RTC_Hours * 60 * 60)
    + (time->RTC_Minutes * 60)
    + time->RTC_Seconds;
}

void AlarmGet(RTCTime_t* time)
{
  uint32_t RTC_Alarm = alarm_val;
  uint32_t tmp;
  
  RTC_Alarm %= (60*60*24);
  
  tmp = RTC_Alarm % 60;
  RTC_Alarm /= 60;
  
  time->RTC_Seconds = tmp;
  
  tmp = RTC_Alarm % 60;
  RTC_Alarm /= 60;
  
  time->RTC_Minutes = tmp;
  
  time->RTC_Hours = RTC_Alarm;
}

int AlarmIsOn(void)
{
  return alarm_IsOn;
}

void AlarmOn(void)
{
  alarm_IsOn = 1;
}

void AlarmOff(void)
{
  alarm_IsOn = 0;
}

void BackupWrite(uint8_t address, uint16_t val)
{
  if(address > 9)
    return;
  
  flag_BackupIsInit = 1;
  
  __IO uint32_t *pntr = &BKP->DR1;
  *(pntr + address) = val;
}

uint16_t BackupRead(uint8_t address)
{
  if(address > 9)
    return 0;
  
  __IO uint32_t *pntr = &BKP->DR1;
  uint16_t val = *(pntr + address);
  
  return val;
}

int BackupIsInit(void)
{
  return flag_BackupIsInit;
}

//прерывание от RTC
void RTC_IRQHandler(void)
{
  /*
  while(!(RTC->CRL & RTC_CRL_RTOFF))
    ;
  */
  //сбрасываем флаг прерывания
  RTC->CRL &= ~RTC_CRL_SECF; 
  
  uint32_t tmp = RTC->CNTH;
  tmp<<=16;
  tmp |= RTC->CNTL;
  rtc_cnt = tmp;
  rtc_update_flag = 1;
  
  if(alarm_IsOn)
  {
    if(alarm_val == (rtc_cnt % (60*60*24)))
      alarm_status = 1;
  }
}


