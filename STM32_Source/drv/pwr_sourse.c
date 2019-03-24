#include <stdint.h>
#include "stm32f1xx.h"
#include "pwr_sourse.h"

/*
  Sourse detector - PB12
*/

void PSourseInit(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
  GPIOB->CRH &= ~(GPIO_CRH_CNF12 | GPIO_CRH_MODE12);
  
  //Input with pull-up / pull-down
  GPIOB->CRH |= (0x02 << GPIO_CRH_CNF12_Pos);
  GPIOB->ODR |= (1<<12); //pull-up enable
}

//Определение источника питания, пока не используется
uint8_t PSourseIsExternal(void)
{
  return 1;
#if 0
  if(GPIOB->IDR & (1<<12)) //Если в единице
    return 0; //то питание от батарейки
  return 1; //а иначе от сети
#endif
}


