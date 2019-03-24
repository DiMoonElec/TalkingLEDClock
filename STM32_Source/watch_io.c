#include <stdint.h>
#include "stm32f1xx.h"
#include "watch_io.h"

/*****************************************************************/
/************************* Private functuins *********************/
/*****************************************************************/

// Кнопки

#define BUTTON_1_MSK    (1<<3)
#define BUTTON_2_MSK    (1<<4)
#define BUTTON_3_MSK    (1<<5)
#define BUTTON_4_MSK    (1<<8)
#define BUTTON_5_MSK    (1<<7)

static uint8_t ButtonPress = 0;

static uint8_t Button_tmp;
static uint8_t fsmb_state = 0;

static int btn_counter = 0;
static int btn_reload_val = 0;
#define BTN_START_CNTR_VAL      (80 * 2)
#define BTN_END_CNTR_VAL        (15 * 2) 
#define BTN_DECR_STEP           (8 * 2)

//для каких кнопок включен автоповтор
static uint8_t btn_repeat_flags = 0; 

static void ButtonsInit(void)
{
  fsmb_state = 0;
  ButtonPress = 0;
  
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
}

static uint16_t ButtonStatus(void)
{
  uint16_t tmp = GPIOB->IDR; //сохраняем значение порта
  
  tmp ^= 0xFFFF;
  
  if(tmp & BUTTON_1_MSK)
    return BUTTON_1;
  else if(tmp & BUTTON_2_MSK)
    return BUTTON_2;
  else if(tmp & BUTTON_3_MSK)
    return BUTTON_3;
  else if(tmp & BUTTON_4_MSK)
    return BUTTON_4;
  else if(tmp & BUTTON_5_MSK)
    return BUTTON_5;
  else
    return 0;
}




/*****************************************************************/
//Буфер и функции LED-дисплея

//Куда подключены выводы 74hc595
#define _SEG_A   1
#define _SEG_B   2
#define _SEG_C   3
#define _SEG_D   4
#define _SEG_E   5
#define _SEG_F   6
#define _SEG_G   7
#define _SEG_H   0

//Перевернутый вариант
#define SEG_A   (1<<_SEG_D)
#define SEG_B   (1<<_SEG_E)
#define SEG_C   (1<<_SEG_F)
#define SEG_D   (1<<_SEG_A)
#define SEG_E   (1<<_SEG_B)
#define SEG_F   (1<<_SEG_C)
#define SEG_G   (1<<_SEG_G)
#define SEG_H   (1<<_SEG_H)

const static uint8_t LED_SYMBOLS[] = {
/* 0:  '0' */  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)^0xFF,
/* 1:  '1' */  (SEG_B | SEG_C)^0xFF,
/* 2:  '2' */  (SEG_A | SEG_B | SEG_D | SEG_E | SEG_G)^0xFF,
/* 3:  '3' */  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G)^0xFF,
/* 4:  '4' */  (SEG_B | SEG_C | SEG_F | SEG_G)^0xFF,
/* 5:  '5' */  (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)^0xFF,
/* 6:  '6' */  (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)^0xFF,
/* 7:  '7' */  (SEG_A | SEG_B | SEG_C)^0xFF,
/* 8:  '8' */  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)^0xFF,
/* 9:  '9' */  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)^0xFF,
/* 10: ' ' */  (0)^0xFF,
/* 11: '-' */  (SEG_G)^0xFF,
/* 12: '|_'*/  (SEG_D | SEG_E)^0xFF,
/* 13: '¬' */  (SEG_A | SEG_F)^0xFF,
/* 14: '°' */  (SEG_A | SEG_B | SEG_F | SEG_G)^0xFF,
/* 15: 'A' */  (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)^0xFF,
/* 16: 'C' */  (SEG_A | SEG_D | SEG_E | SEG_F)^0xFF,
/* 17: 'U' */  (SEG_C | SEG_D | SEG_E)^0xFF,
/* 18: 'T' */  (SEG_D | SEG_E | SEG_F | SEG_G)^0xFF,
/* 19: 'O' */  (SEG_C | SEG_D | SEG_E | SEG_G)^0xFF,
/* 20: 'B' */  (SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)^0xFF,
/* 21: 'D' */  (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)^0xFF,
/* 22: 'I' */  (SEG_E | SEG_F)^0xFF
};

static uint8_t LED_Buffer[6];
static uint16_t LED_Points_flags = 0;
static uint16_t LED_DigitsBlink_flags = 0;

static uint8_t LED_Off_flag = 0;

#define LED_BLINK_CONST         (128 * 2)
#define LED_BLINK_NSTART        (32 * 2)
static uint16_t blink_counter = 0;

static void LED_Init(void)
{
  for(int i=0; i<sizeof(LED_Buffer); i++)
    LED_Buffer[i] = 0xFF;
}



/*****************************************************************/
//Драйвер 74HC595


#define HC595_RCLK_ACTIVE()     GPIOA->BSRR = (1<<8)
#define HC595_RCLK_INACTIVE()   GPIOA->BRR = (1<<8)
/*
#define HC595_SRCLR_ACTIVE()    GPIOA->BRR = (1<<9)
#define HC595_SRCLR_INACTIVE()  GPIOA->BSRR = (1<<9)
*/

#define HC595_EN_ACTIVE()    GPIOA->BRR = (1<<10)
//#define HC595_EN_INACTIVE()  GPIOA->BSRR = (1<<10)

static void HC595_Init(void)
{
  //Включаем тактирование GPIOB и SPI2
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
  RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
  
  /**********************************************************/
  /*** Настройка выводов GPIOB на работу совместно с SPI2 ***/
  /**********************************************************/
  
  //PB15 - MOSI
  //PB14 - MISO
  //PB13 - SCK
  
  //Для начала сбрасываем все конфигурационные биты в нули
  GPIOB->CRH &= ~(GPIO_CRH_CNF15_Msk | GPIO_CRH_MODE15_Msk 
                | GPIO_CRH_CNF14_Msk | GPIO_CRH_MODE14_Msk 
                | GPIO_CRH_CNF13_Msk | GPIO_CRH_MODE13_Msk);
  
  //MOSI: MODE15 = 0x03 (11b); CNF15 = 0x02 (10b)
  GPIOB->CRH |= (0x02<<GPIO_CRH_CNF15_Pos) | (0x03<<GPIO_CRH_MODE15_Pos);
  
  //MISO: MODE14 = 0x00 (00b); CNF14 = 0x01 (01b)
  GPIOB->CRH |= (0x00<<GPIO_CRH_CNF14_Pos) | (0x01<<GPIO_CRH_MODE14_Pos);
  
  //SCK: MODE13 = 0x03 (11b); CNF13 = 0x02 (10b)
  GPIOB->CRH |= (0x02<<GPIO_CRH_CNF13_Pos) | (0x03<<GPIO_CRH_MODE13_Pos);
  
  /**********************/
  /*** Настройка SPI2 ***/
  /**********************/
  
  SPI2->CR1 = 0<<SPI_CR1_DFF_Pos  //Размер кадра 8 бит
    | 0<<SPI_CR1_LSBFIRST_Pos     //MSB first
    | 1<<SPI_CR1_SSM_Pos          //Программное управление SS
    | 1<<SPI_CR1_SSI_Pos          //SS в высоком состоянии
    | 0x04<<SPI_CR1_BR_Pos        //Скорость передачи: F_PCLK/32
    | 1<<SPI_CR1_MSTR_Pos         //Режим Master (ведущий)
    | 0<<SPI_CR1_CPOL_Pos | 0<<SPI_CR1_CPHA_Pos; //Режим работы SPI: 0
  
  NVIC_EnableIRQ(SPI2_IRQn); //Разрешаем прерывания от SPI2
  
  SPI2->CR1 |= 1<<SPI_CR1_SPE_Pos; //Включаем SPI
  
  /*************************************/
  /*** Настройка управляющих выводов ***/
  /*************************************/
  
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
  
  ///PA8 - RCLK
  ///PA10 - EN

  GPIOA->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_MODE8
                | GPIO_CRH_CNF10 | GPIO_CRH_MODE10);
  
  GPIOA->CRH |= (1<<GPIO_CRH_MODE8_Pos | 1<<GPIO_CRH_MODE10_Pos);
  
  /*
  HC595_SRCLR_ACTIVE();
  HC595_SRCLR_INACTIVE();
  */
  
  HC595_RCLK_ACTIVE();
  HC595_RCLK_INACTIVE();
  
  HC595_EN_ACTIVE();
}


#define SPI_LEN_BUFF    3

static int32_t tx_index = 0;
static uint8_t tx_data[SPI_LEN_BUFF];

static void LED_SPI_Tx(void)
{
  //Ждем, пока SPI освободится от предыдущей передачи
  while(SPI2->SR & SPI_SR_BSY)
    ;
  
  tx_index = 0;
    
  SPI2->CR2 |= (1<<SPI_CR2_TXEIE_Pos); 
}


void SPI2_IRQHandler(void)
{
  SPI2->DR = tx_data[tx_index]; //Записываем новое значение в DR
  tx_index++; //увеличиваем счетчик переданных байт на единицу
  
  //если все передали, то отключаем прерывание
  if(tx_index >= SPI_LEN_BUFF)
  {
    SPI2->CR2 &= ~(1<<SPI_CR2_TXEIE_Pos); 
    
  }
}

/*****************************************************************/

//LED Таймер


static uint8_t led_pwm = 9;

static void LEDTimer_init(void)
{ 
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; //Включаем тактирование TIM3
  
  TIM3->PSC = 71; //Предделитель 71+1=72
  TIM3->ARR = 166; //период 166мкс, частота 6000Гц //Период 303мкс, частота ~3300Гц
  TIM3->DIER = TIM_DIER_UIE; //Прерывание по обновлению
  
  TIM3->SR &= ~(TIM_SR_UIF); //Очищаем флаг прерывания
  NVIC_EnableIRQ(TIM3_IRQn); //Разрешаем прерывание от таймера
  TIM3->CR1 |= TIM_CR1_CEN; //Включаем таймер
}

static void led_delay(void)
{
  for(int i=0; i<2; i++)
    asm("nop");
}

//Прерывание TIM3
void TIM3_IRQHandler(void)
{
  static uint8_t counter = 0;
  static uint8_t led_index = 0;
  static uint8_t tmp = 0;
  static uint8_t _led_off_flag = 0;
  static uint8_t fsm_on_off = 0;
  TIM3->SR &= ~(TIM_SR_UIF); //Очищаем флаг прерывания
  
  /////////////////////////////////////////////////////////////////
  
  if(_led_off_flag == 0)
  {
    if(counter == 0)
    {
      //Отправляем значение в регистры
      
      /////////////////////////////////////
      // Первый индикатор
      if((1<<led_index) & LED_DigitsBlink_flags) //Если включено мигание сегмента
      {
        if(blink_counter > (LED_BLINK_NSTART + LED_BLINK_CONST/2))
          tmp = LED_SYMBOLS[LED_SYMBOL_NULL];
        else
          tmp = LED_Buffer[led_index];
      }
      else
      {  
        tmp = LED_Buffer[led_index];
      }
      
      if((1<<led_index) & LED_Points_flags) //если включена точка
        tmp &= ~(1<<0); //то включаем ее
      
      tx_data[2] = tmp;
      
      /////////////////////////////////////
      // Второй индикатор
      if((1<<(led_index+3)) & LED_DigitsBlink_flags) //Если включено мигание сегмента
      {
        if(blink_counter > (LED_BLINK_NSTART + LED_BLINK_CONST/2))
          tmp = LED_SYMBOLS[LED_SYMBOL_NULL];
        else
          tmp = LED_Buffer[led_index + 3];
      }
      else
      {
        tmp = LED_Buffer[led_index + 3];
      }
      
      if((1<<(led_index+3)) & LED_Points_flags) //если включена точка
        tmp &= ~(1<<0); //то включаем ее
      
      tx_data[1] = tmp;
      
      
      /////////////////////////////////////
      // Зажигаем нужные цифры
      // и боковые точки
      
      tmp = (8>>led_index)^0xFF;
      
      //Боковые точки висят на 3-м сдивговом регистре
      //поэтому самое время вспомнить о них
      if(LED_POINT_ALARM & LED_Points_flags)
        tmp &= ~(1<<5);
      if(LED_POINT_INFO & LED_Points_flags)
        tmp &= ~(1<<4);
      
      tx_data[0] = tmp;
      
      LED_SPI_Tx();
      
      
      led_index++;
      if(led_index >= 3)
        led_index = 0;
      
      blink_counter++;
      if(blink_counter >= (LED_BLINK_NSTART + LED_BLINK_CONST))
        blink_counter = LED_BLINK_NSTART;
    }
    else if(counter == 1)
    {
      //Проталкиваем значение на выход
      HC595_RCLK_ACTIVE();
      led_delay();
      HC595_RCLK_INACTIVE();
      led_delay();
      
      tx_data[2] = 0xFF;
      tx_data[1] = 0xFF;
      tx_data[0] = 0xFF;
      LED_SPI_Tx();
      
    }
    else if(counter == led_pwm)
    {
        HC595_RCLK_ACTIVE();
        led_delay();
        HC595_RCLK_INACTIVE();
    }
  }
  
  
  counter++;
  if(counter >= 20)
  {
    counter = 0;
    
    //Конечный автомат кнопок
    switch(fsmb_state)
    {
    /*************************************/
    case 0:
      Button_tmp = ButtonStatus();
      if(Button_tmp != 0) //если нажата какая-либо кнопка
        fsmb_state = 1;
      break;
      
    /*************************************/
    case 1:
      if(Button_tmp == ButtonStatus()) //если кнопка по прежнему нажата
      {
        ButtonPress = Button_tmp; //сохраняем нажатую кнопку
         
        //инициализация переменных для автоповтора
        btn_counter = BTN_START_CNTR_VAL;
        btn_reload_val = BTN_START_CNTR_VAL - BTN_DECR_STEP;
        
        //переходим в следующее состояние
        fsmb_state = 2;
      }
      else
      {
        fsmb_state = 0;
      }
      break;
      
    /*************************************/  
    case 2:
      if(ButtonStatus() == 0) //если все отпустили
      {
        fsmb_state = 0; //возвращаемся назад
      }
      else
      {
        //тут реализован автоповтор
        if(btn_counter <= 0)
        {
          //перезагружаем счетчик
          btn_counter = btn_reload_val;
          btn_reload_val -= BTN_DECR_STEP;
          if(btn_reload_val < BTN_END_CNTR_VAL)
            btn_reload_val = BTN_END_CNTR_VAL;
         
          //если автоповтор для этой кнопки включен
          if(btn_repeat_flags & (1<<Button_tmp))
            ButtonPress = Button_tmp; // устанавливаем кнопку нажатой
        }
        btn_counter--;
      }
      break;
    }
    /*************************************/
    /*************************************/
    
    //тут логика включения и выключения дисплея
    switch(fsm_on_off)
    {
    case 0:
      if(_led_off_flag != LED_Off_flag) //если изменилось состояние внешнего флага
      {
        _led_off_flag = LED_Off_flag;
        if(_led_off_flag == 1) //если надо выключить
        {
          //отправляем гасящую последовательность
          tx_data[2] = 0xFF;
          tx_data[1] = 0xFF;
          tx_data[0] = 0xFF;
          LED_SPI_Tx();
          fsm_on_off = 1;
        }
        
        //а если надо включить, 
        //то достаточно (_led_off_flag = LED_Off_flag) == 1
        //и все само включится
      }
      break;
      
    case 1:
      //пропихиваем гасящую последовательность
      HC595_RCLK_ACTIVE();
      led_delay();
      HC595_RCLK_INACTIVE();
      fsm_on_off = 0;
      break;
    }
    
  }
}

/*****************************************************************/


/*****************************************************************/
/************************* Public functuins **********************/
/*****************************************************************/

void io_init(void)
{
  LED_Init();
  HC595_Init();
  ButtonsInit();
  LEDTimer_init();
}

void LED_Set(int index, uint8_t val)
{
  if((index >= sizeof(LED_Buffer))
     || (val >= sizeof(LED_SYMBOLS)))
    return;
  
  LED_Buffer[index] = LED_SYMBOLS[val];
}

void LED_BlinkOn(uint8_t n)
{
  LED_DigitsBlink_flags |= n;
}

void LED_BlinkOff(uint8_t n)
{
  LED_DigitsBlink_flags &= ~n;
}

void LED_BlinkOffAll(void)
{
  LED_DigitsBlink_flags = 0;
}

void LED_BlinkTimerReset(void)
{
  blink_counter = 0;
}


void LED_PointsSet(uint8_t p)
{
  LED_Points_flags |= p;
}

void LED_PointsClear(uint8_t p)
{
  LED_Points_flags &= ~p;
}

void LED_Brightness(uint8_t val)
{
  if(val > 17)
    val = 17;
  
  led_pwm = val + 2;
}

void LED_Off(void)
{
  LED_Off_flag = 1;
}

void LED_On(void)
{
  LED_Off_flag = 0;
}

uint8_t LED_Status(void)
{
  return !LED_Off_flag;
}

uint8_t GetButton(void)
{
  uint8_t tmp = ButtonPress;
  ButtonPress = 0;
  return tmp;
}

void ButtonArptOn(uint8_t button)
{
  btn_repeat_flags |= (1<<button);
}

void ButtonArptOff(uint8_t button)
{
  btn_repeat_flags &= ~(1<<button);
}

void ButtonArptOffAll(void)
{
  btn_repeat_flags = 0;
}
