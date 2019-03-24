#include <stdint.h>
#include "stm32f1xx.h"
#include "at45db321.h"
#include "media.h"


/*
  TIM2 - PWM Timer
  PA0 - TIM2_CH1_ETR
  PA1 - TIM2_CH2
  AMP_ENABLE - PB9
*/


static uint8_t flag_IsRdy = 0;
static uint8_t pcm_val;


#define AMP_ENABLE()     GPIOB->BRR = (1<<9)
#define AMP_DISABLE()    GPIOB->BSRR = (1<<9)

static void AmpEnableInit(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
  GPIOB->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
  GPIOB->CRH |= (1<<GPIO_CRH_MODE9_Pos);
}

/************************************************/

static uint32_t total_bytes;
static uint32_t b_cntr;

/************************************************/
/*
  PWM-таймер для вывода звука
*/

#define PWM_A_REG       TIM2->CCR1
#define PWM_B_REG       TIM2->CCR2

static __IO uint32_t *PWM_REG = &PWM_A_REG;


static void PWM_Enable(void)
{
  TIM2->CR1 |= TIM_CR1_CEN;
}

static void PWM_Disable(void)
{
  TIM2->CR1 &= ~TIM_CR1_CEN;
  
  /*
  TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC2M_2; 
  TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC2M_2);
  */
}

static void PWM_Init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //Включаем тактирование порта GPIOA
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //таймера TIM2
  
  //Настройка портов PA0 и PA1
  GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0
                | GPIO_CRL_MODE1 | GPIO_CRL_CNF1);
  
  //Настройка на выход в режиме альтернативной функции
  //PA0
  GPIOA->CRL |= (0x01 << GPIO_CRL_MODE0_Pos
              | 0x02 << GPIO_CRL_CNF0_Pos);
  //PA1
  GPIOA->CRL |= (0x01 << GPIO_CRL_MODE1_Pos
              | 0x02 << GPIO_CRL_CNF1_Pos);
  
  //Настройка таймера TIM2
  TIM2->ARR = 0x00FF; //ШИМ 8 бит
  TIM2->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E; //Таймер управляет выводами
  //TIM2_CH1 и TIM2_CH2
  TIM2->CCMR1 = (0x06 << TIM_CCMR1_OC1M_Pos | TIM_CCMR1_OC1PE
               | 0x06 << TIM_CCMR1_OC2M_Pos | TIM_CCMR1_OC2PE); //Настройка
  //в режим PWM1 выводов TIM2_CH1 и TIM2_CH2
  TIM2->CR1 |= TIM_CR1_ARPE;
  
  PWM_Disable();
  /*
  TIM2->CR1 |= TIM_CR1_CEN;
  
  PWM_A_Set(255/2);
  PWM_B_Set(10);
  */
}

/************************************************/
/*
  Таймер семплов
*/

static void STimer_Init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; //включаем тактирование TIM1
  TIM1->ARR = 3265; //частота ~22050 Гц

  TIM1->SR &= ~TIM_SR_UIF; //на всякий случай сбрасываем флаг прерывания
  TIM1->DIER |= TIM_DIER_UIE; //разрешаем прерывание по обновлению
  NVIC_EnableIRQ(TIM1_UP_IRQn); //Разрешаем прерывания от таймера
}

static void STimer_Enable(void)
{
  TIM1->CR1 |= TIM_CR1_CEN;
}

static void STimer_Disable(void)
{
  TIM1->CR1 &= ~TIM_CR1_CEN;
  flag_IsRdy = 1;
}

/************************************************/

int media_isrdy(void)
{
  /*
  if(TIM1->CR1 & TIM_CR1_CEN)
    return 0;
  return 1;
  */
  return flag_IsRdy;
}

void media_abort(void)
{
  STimer_Disable();
  at45_StreamStop();
}

//запускаем воспроизведение аудиопотока
void media_play(uint32_t start, uint32_t len)
{
  flag_IsRdy = 0;
  at45_StreamStart(start);
  total_bytes = len;
  b_cntr = 0;
  
  pcm_val = at45_StreamRead();
  STimer_Enable();
}

void media_enable(MediaChannel_t channel)
{
  PWM_Enable();
  if(channel == CHANNEL_A)
  {
    PWM_REG = &PWM_A_REG;
    PWM_A_REG = 127;
    PWM_B_REG = 0;
  }
  else
  {
    PWM_REG = &PWM_B_REG;
    PWM_A_REG = 0;
    PWM_B_REG = 127;
  }
  
  AMP_ENABLE();
}

void media_disable(void)
{
  AMP_DISABLE();
  
  PWM_Disable();
}

void media_init(void)
{
  at45_init();
  PWM_Init();
  STimer_Init();
  flag_IsRdy = 1;
  AmpEnableInit();
  AMP_DISABLE();
}

/************************************************/


void TIM1_UP_IRQHandler(void)
{
  TIM1->SR &= ~TIM_SR_UIF;
  
  //PWM_A_REG = pcm_val;
  *PWM_REG = pcm_val;
  
  pcm_val = at45_StreamRead();
  b_cntr++;
  if(b_cntr >= total_bytes)
  {
    STimer_Disable();
    at45_StreamStop();
  }
}
