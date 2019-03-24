#include <stdint.h>
#include "stm32f1xx.h"
#include "timers.h"


static uint32_t timers[MAX_TIMERS];

#define TMR_INT_DISABLE()       SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk
#define TMR_INT_ENABLE()        SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk

static void SysTick_Init(void)
{
  SysTick->LOAD = (90000 - 1);
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void InitTimers(void)
{
  uint32_t i;
  for(i = 0; i < MAX_TIMERS; i++)
    timers[i] = 0;
  
  SysTick_Init();
}

uint32_t GetTimer(uint32_t TimerID)
{
  uint32_t tmp;
  
  TMR_INT_DISABLE();
  tmp = timers[TimerID];
  TMR_INT_ENABLE();
  
  return tmp;
}

void ResetTimer(uint32_t TimerID)
{
  TMR_INT_DISABLE();
  timers[TimerID] = 0;
  TMR_INT_ENABLE();
}

void SysTick_Handler(void)
{
  static uint32_t i = 0;
  
  for(i=0; i<MAX_TIMERS; i++)
    timers[i]++;
}




