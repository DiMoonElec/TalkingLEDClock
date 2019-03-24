//Драйвер LED-дисплея и кнопок
//Использует следуюшие периферийные устройства:
//SPI2
//PB13, PB14, PB15
//PB3, PB4, PB5, PB6, PB7
//PA8, PA9
//TIM3

#ifndef __WATCH_IO_H__
#define __WATCH_IO_H__

#define LED_BRIGHT_MAX      17
#define LED_BRIGHT_MIN      0

#define LED_POINT_0         (1<<0)
#define LED_POINT_1         (1<<1)
#define LED_POINT_2         (1<<2)
#define LED_POINT_3         (1<<3)
#define LED_POINT_4         (1<<4)
#define LED_POINT_5         (1<<5)
#define LED_POINT_ALARM     (1<<6)
#define LED_POINT_INFO      (1<<7)

#define LED_SYMBOL_NULL     10
#define LED_SYMBOL_MINUS    11
#define LED_SYMBOL_INDOOR   12
#define LED_SYMBOL_OUTDOOR  13
#define LED_SYMBOL_GRADUS   14
#define LED_SYMBOL_A        15
#define LED_SYMBOL_C        16
#define LED_SYMBOL_U        17
#define LED_SYMBOL_T        18
#define LED_SYMBOL_O        19
#define LED_SYMBOL_B        20
#define LED_SYMBOL_D        21
#define LED_SYMBOL_I        22

#define LED_BLINK_0         (1<<0)
#define LED_BLINK_1         (1<<1)
#define LED_BLINK_2         (1<<2)
#define LED_BLINK_3         (1<<3)
#define LED_BLINK_4         (1<<4)
#define LED_BLINK_5         (1<<5)
#define LED_BLINK_ALL       (LED_BLINK_0 | LED_BLINK_1 | LED_BLINK_2 | LED_BLINK_3 | LED_BLINK_4 | LED_BLINK_5)


//кнопки
#define BUTTON_1            1
#define BUTTON_2            2
#define BUTTON_3            3
#define BUTTON_4            4
#define BUTTON_5            5

#define BTN_MENU            BUTTON_4
#define BTN_ALARM           BUTTON_5
#define BTN_PLUS            BUTTON_3
#define BTN_MINUS           BUTTON_2
#define BTN_TELL            BUTTON_1


void io_init(void); //инициализация дисплея и кнопок
void LED_Set(int index, uint8_t val); //Установить значение val цифре с номером index
void LED_Brightness(uint8_t val); //Установить яркость индикаторов (0..17)
void LED_BlinkOn(uint8_t n); //Включить мигание n-й цифры
void LED_BlinkOff(uint8_t n); //Выключить мигание n-й цифры
void LED_BlinkOffAll(void); //Выключить мигание всех цифр
void LED_BlinkTimerReset(void); //Сбросить внутренний таймер мигания
void LED_PointsSet(uint8_t p); //Включить p-ю точку 
void LED_PointsClear(uint8_t p); //Выключить p-ю точку 
void LED_Off(void); //Погасить LED-дисплей
void LED_On(void); //Зажечь LED-дисплей
uint8_t LED_Status(void); //Статус дисплея: 1-горит, 0-погашен
uint8_t GetButton(void); //Получить нажатую кнопку
void ButtonArptOn(uint8_t button); //Включить автоповтор кнопки
void ButtonArptOff(uint8_t button); //Выключить автоповтор кнопки
void ButtonArptOffAll(void); //Выключить автоповтор всех кнопок

#endif
