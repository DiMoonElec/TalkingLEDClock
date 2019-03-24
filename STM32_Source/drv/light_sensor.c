#include <stdint.h>
#include "stm32f1xx.h"
#include "timers.h"
#include "light_sensor.h"

/*
  Используемая периферия 
    ADC1
    PA4
*/

#define ADC_ENABLE()    ADC1->CR2 |= ADC_CR2_ADON
#define ADC_DISABLE()   ADC1->CR2 &= ADC_CR2_ADON

#define ADC_CONVERT()    ADC1->CR2 |= ADC_CR2_ADON

#define ADC_DATA         ADC1->DR


#define WIN_SIZE        8
static uint16_t ma_array[WIN_SIZE];
static uint8_t ma_index = 0;

////////////////////////////

static uint8_t state;
//static uint8_t light_val;
static IllumLevel_t light_val;

////////////////////////////

//Инициализация фильтров
static void ma_init(void)
{
  ma_index = 0;
  
  for(int i=0; i<WIN_SIZE; i++)
  {
    ma_array[i] = 0;
  }
}

//Добавить новое значение в фильтр
static void ma_add(uint16_t val)
{
  ma_array[ma_index] = val;
  
  ma_index++;
  if(ma_index >= WIN_SIZE)
    ma_index = 0;
}

//плучить сглаженное значение
static uint16_t ma_get(void)
{
  uint32_t tmp = 0;
  for(int i = 0; i<WIN_SIZE; i++)
  {
    tmp += ma_array[i];
  }
  
  return tmp / WIN_SIZE;
}

///////////////////////////


static void adc_init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  
  //GPIO init
  //PA4 - Analog mode
  GPIOA->CRL &= ~(GPIO_CRL_CNF4 | GPIO_CRL_MODE4);
  
  //ADC1 init
  RCC->CFGR |= (0x03 << RCC_CFGR_ADCPRE_Pos); //предделитель ADC
  ADC1->SQR3 = (0x04 << ADC_SQR3_SQ1_Pos);
  ADC1->SQR1 = 0;
  
  //calibration
  ADC_ENABLE();
  
  for(int i=0; i<100000; i++)
    ;
  ADC1->CR2 |= ADC_CR2_CAL;
  
  while(ADC1->CR2 & ADC_CR2_CAL)
    ;
}


static void ADC2Light(uint16_t adc_val)
{
  //пропускаем значение АЦП через скользящее среднее
  ma_add(adc_val);
  uint32_t ma_val = ma_get();
  
  
  if(ma_val <= ILLUM_LIGHT_H) //проверка границ "Светло"
  {
    light_val = ILLUM_LIGHT; //светло
  }
  else if(ma_val >= ILLUM_MEDIUM_L && ma_val <= ILLUM_MEDIUM_H) //проверка границ "Средне"
  {
    light_val = ILLUM_MEDIUM; //средне
  }
  else if(ma_val >= ILLUM_DARK_L && ma_val <= ILLUM_ADC_MAX_VAL) //проверка границ "Темно"
  {
    light_val = ILLUM_DARK; //темно
  }
  //проверка корректного значения неопределенных областей
  else if((ma_val > ILLUM_LIGHT_H && ma_val < ILLUM_MEDIUM_L) //Если находимся между "Светло" и "Средне", 
          && (light_val != ILLUM_LIGHT && light_val != ILLUM_MEDIUM)) //но предыдущее значение освещенности не "Светло" и не "Средне",
  {
    light_val = ILLUM_LIGHT; //то исправляем это на значение "Светло"
  }
  else if((ma_val > ILLUM_MEDIUM_H && ma_val < ILLUM_DARK_L) //Если между "Средне" и "Темно"
          && (light_val != ILLUM_MEDIUM && light_val != ILLUM_DARK)) //но предыдущее значение не "Средне" и не "Темно"
  {
    light_val = ILLUM_MEDIUM; //то исправляем на "Средне"
  }
           
  
  
  //Старая реализация датчика освещенности
  /*
  ma_add(adc_val);
  
  int32_t tmp = ma_get();
  
  tmp -= light_val*16; //реализация гистерезиса срабатывания уровней
  
  if(tmp > 3999)
    light_val = 0; //темно
  else if(tmp > 2400) //2400
    light_val = 1; //сумерки
  else
    light_val = 2; //светло
  */
}


void InitLigthSensor(void)
{
  adc_init();
  state = 0;
  //light_val = 0;
  light_val = ILLUM_DARK;
  ma_init();
}



void ProcessLigthSensor(void)
{
  switch(state)
  {
  case 0:
    ResetTimer(TIMER_LIGHT_SENSOR);
    ADC_CONVERT();
    state = 1;
    break;
    
  case 1:
    if(GetTimer(TIMER_LIGHT_SENSOR) >= (TMR_SEC / 10))
    {
      ADC2Light(ADC_DATA);
      state = 0;
    }
    break;
    
  default:
    state = 0;
    break;
  }
}

IllumLevel_t LigthSensorVal(void)
{
  return light_val;
}

