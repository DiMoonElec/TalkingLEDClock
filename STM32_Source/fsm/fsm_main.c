#include <stdint.h>
#include "watch_io.h"
#include "rtc.h"

#include "timers.h"
#include "fsm_main.h"
#include "voice.h"
#include "temper.h"
#include "params.h"
#include "pwr_sourse.h"
#include "light_sensor.h"

/************************************************************************/
/************************************************************************/
static RTCTime_t Time;
static RTCTime_t TmpTime;
//static uint8_t tmp1;
//static uint8_t tmp2;
static uint8_t UInt8Tmp;
static int8_t Int8Tmp;
static char gflag;
static uint8_t br = LED_BRIGHT_MAX;
static uint8_t flag_SignalAlarmActive = 0;

static struct SysParams_s SysParams;

#define ALARM_DURATION          300 //300 секунд

/************************************************************************/

static void InfoExe(RTCTime_t *Time, uint8_t variant);

/************************************************************************/

static uint8_t state, _state;
static char entry;


enum
{
  STATE_MAIN = 0,
  STATE_DISP_TEMPER_OUTDOOR,
  STATE_DISP_TEMPER_INDOOR,
  
  PNT_MENU_BEGIN, //начало меню
  /*********************/
  STATE_ALARM,
  STATE_ALARM_DISP,
  STATE_ALARM_SETHH,
  STATE_ALARM_SETMM,
  /*********************/
  STATE_MEINU_BEGIN,
  STATE_SETUP_HOUR,
  STATE_SETUP_MIN,
  
  STATE_SETUP_INFO,
  STATE_SETUP_INFO_BEGIN,
  STATE_SETUP_INFO_END,
  
  STATE_SETUP_ALARM_MELODY,
  
  STATE_SETUP_BRIGHT,
  
  STATE_SETUP_RTC_CORR,
  
  STATE_SETUP_CONFIG,
  STATE_SETUP_CONFIG_DISPLAY,
  STATE_SETUP_CONFIG_INFORM,
  STATE_SETUP_CONFIG_AUTOINFORM,
  
  /*********************/
  PNT_MENU_END, //конец меню
  /*********************/
  STATE_MENU_EXIT  
};

/************************************************************************/
//для конечного автомата управления энергопотреблением
static uint8_t pwr_state = 0;

/************************************************************************/
/*
#define INCR(x, max) {if(x<max) { x++; }}
#define DECR(x, min) {if(x>min) { x--; }}
*/
static unsigned char dec2bcdTens(unsigned char num){return num / 10;}
static unsigned char dec2bcdUnits(unsigned char num){return num % 10;}

/************************************************************************/
/////////////////////////////////////
static void DisplayTime(RTCTime_t* RTC_Time, uint8_t SepStatus)
{
  if(RTC_Time->RTC_Hours > 9)
    LED_Set(0, dec2bcdTens(RTC_Time->RTC_Hours));
  else
    LED_Set(0, LED_SYMBOL_NULL);
  LED_Set(1, dec2bcdUnits(RTC_Time->RTC_Hours));
  LED_Set(2, (SepStatus ? LED_SYMBOL_MINUS : LED_SYMBOL_NULL));
  LED_Set(3, dec2bcdTens(RTC_Time->RTC_Minutes));
  LED_Set(4, dec2bcdUnits(RTC_Time->RTC_Minutes));
  LED_Set(5, LED_SYMBOL_NULL);
}
/////////////////////////////////////
static void DisplayTemper(float temper, uint8_t flag_indoor)
{
  uint8_t flag_minus = 0;
  int8_t t = 0;
  
  t = (int8_t)temper;
  
  if(t < 0)
  {
    flag_minus = 1;
    t = -t;
  }
  
  /*
  if(temper < 0)
  {
    temper = -temper;
    flag_minus = 1;
  }
  t = (uint8_t)temper;
  */
  
  if(flag_indoor)
    LED_Set(0, LED_SYMBOL_INDOOR);
  else
    LED_Set(0, LED_SYMBOL_OUTDOOR);
 LED_Set(1, LED_SYMBOL_NULL); 
 
 if(t > 9)
 {
   LED_Set(2, (flag_minus ? LED_SYMBOL_MINUS : LED_SYMBOL_NULL)); 
   LED_Set(3, dec2bcdTens(t)); 
   LED_Set(4, dec2bcdUnits(t)); 
   LED_Set(5, LED_SYMBOL_GRADUS); 
 }
 else
 {
   LED_Set(2, LED_SYMBOL_NULL); 
   LED_Set(3, (flag_minus ? LED_SYMBOL_MINUS : LED_SYMBOL_NULL)); 
   LED_Set(4, dec2bcdUnits(t)); 
   LED_Set(5, LED_SYMBOL_GRADUS); 
 }
}
/////////////////////////////////////
static void DisplayAlarm(RTCTime_t* RTC_Time)
{
  LED_Set(0, LED_SYMBOL_NULL);
  if(RTC_Time->RTC_Hours > 9)
    LED_Set(1, dec2bcdTens(RTC_Time->RTC_Hours));
  else
    LED_Set(1, LED_SYMBOL_NULL);
  LED_Set(2, dec2bcdUnits(RTC_Time->RTC_Hours));
  LED_Set(3, LED_SYMBOL_MINUS);
  LED_Set(4, dec2bcdTens(RTC_Time->RTC_Minutes));
  LED_Set(5, dec2bcdUnits(RTC_Time->RTC_Minutes));
}
/////////////////////////////////////
static void DisplayVInfo(uint8_t VoiseInfoBegin, uint8_t VoiseInfoEnd, uint8_t VoiseInfoEnable)
{
  LED_Set(0, LED_SYMBOL_NULL);
  
  if(VoiseInfoEnable)
  {
    LED_Set(1, dec2bcdTens(VoiseInfoBegin));
    LED_Set(2, dec2bcdUnits(VoiseInfoBegin));
    LED_Set(3, LED_SYMBOL_MINUS);
    LED_Set(4, dec2bcdTens(VoiseInfoEnd));
    LED_Set(5, dec2bcdUnits(VoiseInfoEnd));
  }
  else
  {
    LED_Set(1, LED_SYMBOL_MINUS);
    LED_Set(2, LED_SYMBOL_MINUS);
    LED_Set(3, LED_SYMBOL_MINUS);
    LED_Set(4, LED_SYMBOL_MINUS);
    LED_Set(5, LED_SYMBOL_MINUS);
  }
}
/////////////////////////////////////
static void DisplayAlarmMelody(uint8_t m)
{
  m++;
  
  LED_Set(0, LED_SYMBOL_A);
  LED_Set(1, LED_SYMBOL_NULL);
  LED_Set(2, LED_SYMBOL_NULL);
  LED_Set(3, LED_SYMBOL_NULL);
  
  if(m > 9)
    LED_Set(4, dec2bcdTens(m));
  else 
    LED_Set(4, LED_SYMBOL_NULL);
  
  LED_Set(5, dec2bcdUnits(m));
}
/////////////////////////////////////
static void DisplayRTCCorr(int8_t m)
{
  uint8_t flag_minus = 0;
  if(m < 0)
  {
    flag_minus = 1;
    m = -m;
  }
  
  LED_Set(0, LED_SYMBOL_C);
  LED_Set(1, LED_SYMBOL_C);
  LED_Set(2, LED_SYMBOL_NULL);
  
  if(m > 9)
  {
    if(flag_minus)
      LED_Set(3, LED_SYMBOL_MINUS);
    else
      LED_Set(3, LED_SYMBOL_NULL);
    LED_Set(4, dec2bcdTens(m));
    LED_Set(5, dec2bcdUnits(m));
  }
  else
  {
    LED_Set(3, LED_SYMBOL_NULL);
    if(flag_minus)
      LED_Set(4, LED_SYMBOL_MINUS);
    else
      LED_Set(4, LED_SYMBOL_NULL);
    LED_Set(5, dec2bcdUnits(m));
  }
}
/////////////////////////////////////
static void DisplayBrVal(uint8_t b)
{
  LED_Set(0, LED_SYMBOL_B);
  LED_Set(1, LED_SYMBOL_NULL);
  
  if(b == 0)
  {
    LED_Set(2, LED_SYMBOL_A);
    LED_Set(3, LED_SYMBOL_U);
    LED_Set(4, LED_SYMBOL_T);
    LED_Set(5, LED_SYMBOL_O);
  }
  else
  {
    if(b > 9)
    {
      LED_Set(2, LED_SYMBOL_NULL);
      LED_Set(3, LED_SYMBOL_NULL);
      LED_Set(4, dec2bcdTens(b));
      LED_Set(5, dec2bcdUnits(b));
    }
    else
    {
      LED_Set(2, LED_SYMBOL_NULL);
      LED_Set(3, LED_SYMBOL_NULL);
      LED_Set(4, LED_SYMBOL_NULL);
      LED_Set(5, dec2bcdUnits(b));
    }
  }
}
/////////////////////////////////////
static void DisplayConfigDisplay(uint8_t ConfigDisplay, 
                                 uint8_t ConfigInform, 
                                 uint8_t ConfigAutoInform,
                                 uint8_t CurrentParam)
{
  if(CurrentParam == 0) //ConfigDisplay
  {
    LED_Set(0, LED_SYMBOL_D);
    LED_Set(1, LED_SYMBOL_NULL);
  }
  else if(CurrentParam == 1) //ConfigInform
  {
    LED_Set(0, LED_SYMBOL_I);
    LED_Set(1, LED_SYMBOL_NULL);
  }
  else //ConfigAutoInform
  {
    LED_Set(0, LED_SYMBOL_A);
    LED_Set(1, LED_SYMBOL_I);
  }
  
  LED_Set(2, LED_SYMBOL_NULL);
  
  LED_Set(3, ConfigDisplay);
  LED_Set(4, ConfigInform);
  LED_Set(5, ConfigAutoInform);
  
}
/************************************************************************/
/////////////////////////////////////
static void PointsClear(void)
{
  LED_PointsClear(LED_POINT_0 | LED_POINT_1 
                | LED_POINT_2 | LED_POINT_3 
                | LED_POINT_4 | LED_POINT_5);
}
/////////////////////////////////////
static void PointsTimeSetup(void)
{
  LED_PointsSet(LED_POINT_0);
  
  LED_PointsClear(LED_POINT_1 | LED_POINT_2 
                | LED_POINT_3 | LED_POINT_4 
                | LED_POINT_5);
}
/////////////////////////////////////
static void PointsVInfo(void)
{
  LED_PointsClear(LED_POINT_0 | LED_POINT_1 
                | LED_POINT_2 | LED_POINT_3 
                | LED_POINT_4 | LED_POINT_5);
  
  LED_PointsSet(LED_POINT_1);
}
/////////////////////////////////////
static void PointsAlarmMelody(void)
{
  LED_PointsClear(LED_POINT_0 | LED_POINT_1 
                | LED_POINT_2 | LED_POINT_3 
                | LED_POINT_4 | LED_POINT_5);
  
  LED_PointsSet(LED_POINT_2);
}
/////////////////////////////////////
static void PointsBrVal(void)
{
  LED_PointsClear(LED_POINT_0 | LED_POINT_1 
                | LED_POINT_2 | LED_POINT_3 
                | LED_POINT_4 | LED_POINT_5);
  
  LED_PointsSet(LED_POINT_3);
}
/////////////////////////////////////
static void PointsRTCCorr(void)
{
  LED_PointsClear(LED_POINT_0 | LED_POINT_1 
                | LED_POINT_2 | LED_POINT_3 
                | LED_POINT_4 | LED_POINT_5);
  
  LED_PointsSet(LED_POINT_4);
}
/////////////////////////////////////
static void PointsConfig(void)
{
  LED_PointsClear(LED_POINT_0 | LED_POINT_1 
                | LED_POINT_2 | LED_POINT_3 
                | LED_POINT_4 | LED_POINT_5);
  
  LED_PointsSet(LED_POINT_5);
}


/************************************************************************/
static void FlickerAlarmHH(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_1);
  LED_BlinkOn(LED_BLINK_2);
}
/////////////////////////////////////
static void FlickerAlarmMM(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_4);
  LED_BlinkOn(LED_BLINK_5);
}
/////////////////////////////////////
static void FlickerVInfoStatus(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_1);
  LED_BlinkOn(LED_BLINK_2);
  LED_BlinkOn(LED_BLINK_3);
  LED_BlinkOn(LED_BLINK_4);
  LED_BlinkOn(LED_BLINK_5);
}
/////////////////////////////////////
static void FlickerVInfoBegin(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_1);
  LED_BlinkOn(LED_BLINK_2);
}
/////////////////////////////////////
static void FlickerVInfoEnd(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_4);
  LED_BlinkOn(LED_BLINK_5);
}
/////////////////////////////////////
static void FlickerAlarmMelody(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_4);
  LED_BlinkOn(LED_BLINK_5);
}
/////////////////////////////////////
static void FlickerBrVal(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_2);
  LED_BlinkOn(LED_BLINK_3);
  LED_BlinkOn(LED_BLINK_4);
  LED_BlinkOn(LED_BLINK_5);
}
/////////////////////////////////////
static void FlickerRTCCorr(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_3);
  LED_BlinkOn(LED_BLINK_4);
  LED_BlinkOn(LED_BLINK_5);
}
/////////////////////////////////////
static void FlickerConfigDisplay(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_3);
}

static void FlickerConfigInform(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_4);
}

static void FlickerConfigAutoInform(void)
{
  LED_BlinkOffAll();
  LED_BlinkOn(LED_BLINK_5);
}
/////////////////////////////////////


/*
  Алгоритм запуска речевого информатора 
  позаимствован из первой версии говорящих частов
  tc v1.0
*/
static void AutoInfoTry(RTCTime_t *t)
{
  if(t->RTC_Seconds == 0
     && t->RTC_Minutes == 0
     && SysParams.VoiseInfoEnable)
  {
    if(SysParams.VoiseInfoBegin == SysParams.VoiseInfoEnd)
    {
      //Time2Voice(t->RTC_Hours, t->RTC_Minutes);
      InfoExe(t, SysParams.ConfigAutoInform);
    }
    else if(SysParams.VoiseInfoBegin < SysParams.VoiseInfoEnd)
    {
      if(t->RTC_Hours >= SysParams.VoiseInfoBegin
         && t->RTC_Hours <= SysParams.VoiseInfoEnd)
      {
        InfoExe(t, SysParams.ConfigAutoInform);
        //Time2Voice(t->RTC_Hours, t->RTC_Minutes);
      }
    }
    else if(!(t->RTC_Hours < SysParams.VoiseInfoBegin
              && t->RTC_Hours > SysParams.VoiseInfoEnd))
    {
      InfoExe(t, SysParams.ConfigAutoInform);
      //Time2Voice(t->RTC_Hours, t->RTC_Minutes);
    }
  }
}

static void AlarmTry(void)
{
  if(Alarm_Triggered())
  {
    uint32_t tmp = GetAlarmLen(SysParams.AlarmMelody);
    if(tmp > 0)
    {
      tmp = (ALARM_DURATION * SAMPLE_RATE) / tmp;
      tmp += 1;
    }
    else
    {
      tmp = 10;
    }
    AlarmPlay(SysParams.AlarmMelody, tmp);
    flag_SignalAlarmActive = 1;
  }
}
//SysParams.ConfigInform
static void InfoExe(RTCTime_t *Time, uint8_t variant)
{
  /////////////////////////////////////////////////
  if(variant == 0) //только время
  {
    Time2Voice(Time->RTC_Hours, Time->RTC_Minutes);
  }
  /////////////////////////////////////////////////
  else if(variant == 1) //время + темпер. снаружи
  {
    if(GetTemperStatus(OUTDOOR_SENSOR) != -1)
    {
      int8_t t = (int8_t)GetTemper(OUTDOOR_SENSOR);
      uint8_t flag_minus;
      if(t < 0)
      {
        flag_minus = 1;
        t = -t;
      }
      else 
        flag_minus = 0;
      
      TimeTemper2Voice(Time->RTC_Hours, Time->RTC_Minutes,
                       t, flag_minus, 1);
    }
    else 
    {
      Time2Voice(Time->RTC_Hours, Time->RTC_Minutes);
    }
  }
  /////////////////////////////////////////////////
  else //время + темпер. внутри
  {
    if(GetTemperStatus(INDOOR_SENSOR) != -1)
    {
      int8_t t = (int8_t)GetTemper(INDOOR_SENSOR);
      uint8_t flag_minus;
      if(t < 0)
      {
        flag_minus = 1;
        t = -t;
      }
      else 
        flag_minus = 0;
      
      TimeTemper2Voice(Time->RTC_Hours, Time->RTC_Minutes,
                       t, flag_minus, 0);
    }
    else 
    {
      Time2Voice(Time->RTC_Hours, Time->RTC_Minutes);
    }
  }
}

/********************************************/

#define CORR_HOUR       23 //в 23 часа
#define CORR_MIN        57 //57 минут
#define CORR_SEC        30 //30 секунд

#define CORR_PERIOD     10 //каждые 10 дней

static uint8_t corr_cntr = 0; //счетчик дней
//static int8_t corr_constant = 0; //корректировка 
#define RTC_CORR_CNSNT          (SysParams.ClockCorrCnsnt)

static void RTCCorrTry(RTCTime_t* time)
{
  if(time->RTC_Hours == (CORR_HOUR - 1)
     && time->RTC_Minutes == CORR_MIN
     && time->RTC_Seconds == CORR_SEC)
  {
    corr_cntr++;
  }
  else if(time->RTC_Hours == CORR_HOUR
     && time->RTC_Minutes == CORR_MIN
     && time->RTC_Seconds == CORR_SEC)
  {
    if(corr_cntr >= CORR_PERIOD)
    {
      corr_cntr = 0;
      time->RTC_Seconds += RTC_CORR_CNSNT;
      RTC_SetTime(time);
    }
  }
}

/************************************************************************/

/*
static void IncrInt8(int8_t *var, int8_t min, int8_t max)
{
  if((*var) < max)
    (*var)++;
  else
    (*var) = min;
}

static void DecrInt8(int8_t *var, int8_t min, int8_t max)
{
  if((*var) > min)
    (*var)--;
  else
    (*var) = max;
     
}
*/

static void IncrUInt8(uint8_t *var, uint8_t min, uint8_t max)
{
  if((*var) < max)
    (*var)++;
  else
    (*var) = min;
}

static void DecrUInt8(uint8_t *var, uint8_t min, uint8_t max)
{
  if((*var) > min)
    (*var)--;
  else
    (*var) = max;
     
}


static void UncircIncrUInt8(uint8_t *var, uint8_t max)
{
  if((*var) < max)
    (*var)++;
}

static void UncircDecrUInt8(uint8_t *var, uint8_t min)
{
  if((*var) > min)
    (*var)--;
}


static void UncircIncrInt8(int8_t *var, int8_t max)
{
  if((*var) < max)
    (*var)++;
}

static void UncircDecrInt8(int8_t *var, int8_t min)
{
  if((*var) > min)
    (*var)--;
}


/************************************************************************/

enum 
{
  CMSTATE_MAIN = 0,
  CMSTATE_DISP_TIME,
  
  CMSTATE_DISP_TIME_TOUT,
  CMSTATE_DISP_TIME_TOUT_1,
  
  CMSTATE_DISP_TIME_TIN,
  CMSTATE_DISP_TIME_TIN_1,
  
  CMSTATE_DISP_TIME_TOUT_TIN,
  CMSTATE_DISP_TIME_TOUT_TIN_1,
  CMSTATE_DISP_TIME_TOUT_TIN_2
  
};

/*
реализация конечного автомата главного экрана
  *sec_cntr - ссылка на секундынй счетчик, 
который увеличивется синхорнно с тиками RTC.
Используется для измерения временных интервалов 
показа информации на дисплее
  *Time - текущее время RTC
  flag_sep - включен ли разделитель часов и минут
  flag_reset - если установлен в 1, то производится сброс
конечного автомата и возврат из функции. Необходимо вызывать
после выхода из меню в основное состояние
*/

static void state_main(uint16_t *sec_cntr, 
                       RTCTime_t *Time, 
                       uint8_t flag_sep,
                       uint8_t flag_reset)
{
  static uint8_t state = CMSTATE_MAIN;
  static uint8_t _state = CMSTATE_MAIN + 1;
  static uint8_t entry = 0;
  
  if(flag_reset)
  {
    state = CMSTATE_MAIN;
    _state = CMSTATE_MAIN + 1;
    entry = 0;
    return;
  }
  
  if(_state != state)
    entry = 1;
  else 
    entry = 0;
  _state = state;
  
  switch(state)
  {
  case CMSTATE_MAIN:
    if(SysParams.ConfigDisplay == 0) //Только время
      state = CMSTATE_DISP_TIME;
    else if(SysParams.ConfigDisplay == 1) //Время + темпер. снаружи
      state = CMSTATE_DISP_TIME_TOUT;
    else if(SysParams.ConfigDisplay == 2) //Время + темпер. внутри
      state = CMSTATE_DISP_TIME_TIN;
    else if(SysParams.ConfigDisplay == 3) //Время + темпер. снаружи + темпер. внутри
      state = CMSTATE_DISP_TIME_TOUT_TIN;
    else
      state = CMSTATE_DISP_TIME;
    break;
  /***********************************************/
  case CMSTATE_DISP_TIME:
    DisplayTime(Time, flag_sep);
    state = CMSTATE_MAIN;
    break;
  /***********************************************/
  case CMSTATE_DISP_TIME_TOUT:
    if(entry)
    {
      (*sec_cntr) = 0;
    }
    DisplayTime(Time, flag_sep);
    if((*sec_cntr) >= 5)
      state = CMSTATE_DISP_TIME_TOUT_1;
    break;
    
  case CMSTATE_DISP_TIME_TOUT_1:
    if(entry)
    {
      (*sec_cntr) = 0;
    }
    
    if(GetTemperStatus(OUTDOOR_SENSOR) != -1)
      DisplayTemper(GetTemper(OUTDOOR_SENSOR), 0);   
    else
      state = CMSTATE_MAIN;
    
    if((*sec_cntr) >= 2)
      state = CMSTATE_MAIN;
    break;
  /***********************************************/
  case CMSTATE_DISP_TIME_TIN:
    if(entry)
    {
      (*sec_cntr) = 0;
    }
    DisplayTime(Time, flag_sep);
    if((*sec_cntr) >= 5)
      state = CMSTATE_DISP_TIME_TIN_1;
    break;
    
  case CMSTATE_DISP_TIME_TIN_1:
    if(entry)
    {
      (*sec_cntr) = 0;
    }
    
    if(GetTemperStatus(INDOOR_SENSOR) != -1)
      DisplayTemper(GetTemper(INDOOR_SENSOR), 1);   
    else
      state = CMSTATE_MAIN;
    
    if((*sec_cntr) >= 2)
      state = CMSTATE_MAIN;
    break;
  /***********************************************/
  case CMSTATE_DISP_TIME_TOUT_TIN:
    if(entry)
    {
      (*sec_cntr) = 0;
    }
    DisplayTime(Time, flag_sep);
    if((*sec_cntr) >= 5)
      state = CMSTATE_DISP_TIME_TOUT_TIN_1;
    break;
    
  case CMSTATE_DISP_TIME_TOUT_TIN_1:
    if(entry)
    {
      (*sec_cntr) = 0;
    }    
    if(GetTemperStatus(OUTDOOR_SENSOR) != -1)
      DisplayTemper(GetTemper(OUTDOOR_SENSOR), 0);   
    else
      state = CMSTATE_DISP_TIME_TOUT_TIN_2;
    
    if((*sec_cntr) >= 2)
      state = CMSTATE_DISP_TIME_TOUT_TIN_2;
    break;
    
  case CMSTATE_DISP_TIME_TOUT_TIN_2:
    if(entry)
    {
      (*sec_cntr) = 0;
    }
    
    if(GetTemperStatus(INDOOR_SENSOR) != -1)
      DisplayTemper(GetTemper(INDOOR_SENSOR), 1);   
    else
      state = CMSTATE_MAIN;
    
    if((*sec_cntr) >= 2)
      state = CMSTATE_MAIN;
    break;
  /***********************************************/
  }
}

void InitFSMMain(void)
{
  //Инициализация конечного автомата
  state = 0;
  _state = state + 1;
  entry = 1;
  
  pwr_state = 0;
  
  corr_cntr = 0;
  //Инициализация детектора пиатния
  PSourseInit();
  
  LED_Brightness(br);
  
  ParamsLoad(&SysParams); //загружаем параметры часов
  
  //Загружаем будильник
  TmpTime.RTC_Hours = SysParams.AlarmHH;
  TmpTime.RTC_Minutes = SysParams.AlarmMM;
  TmpTime.RTC_Seconds = 0;
  AlarmSet(&TmpTime);
  if(SysParams.AlarmIsEnable)
    AlarmOn();
  else
    AlarmOff();
  
  //SysParams.ConfigDisplay = 3;
}



void ProcessFSMMain(void)
{
  static uint8_t flag_sep = 0;
  //static uint8_t flag_common = 0;
  static uint16_t sec_cntr = 0;
  
  if(_state != state)
    entry = 1;
  else 
    entry = 0;
  
  _state = state;
    
  uint8_t button = GetButton();
  
  
  if(button != 0)
    ResetTimer(TIMER_FSMMAIN_TIMEOUT);
  
  if(state > PNT_MENU_BEGIN && state < PNT_MENU_END &&
     GetTimer(TIMER_FSMMAIN_TIMEOUT) >= TMR_SEC * 60)
  {
    state = STATE_MENU_EXIT;
    ResetTimer(TIMER_FSMMAIN_TIMEOUT);
  }
  
  /**********************************/
  //Если будильник включен, то ставим 
  //точку включенного будильника    
  if(AlarmIsOn())
    LED_PointsSet(LED_POINT_ALARM);
  else
    LED_PointsClear(LED_POINT_ALARM);
  
  if(flag_SignalAlarmActive) //если сейчас звучит будильник
  {
    if(VoiseIsReady())
      flag_SignalAlarmActive = 0;
    else if(button != 0) //нажали на любую кнопку
    {
      VoiceAbort();
      flag_SignalAlarmActive = 0;
      button = 0;
    }
  }
  
  
  /************************************************/
  /*Конечный автомат управления энегопотреблением**/
  /************************************************/
  if(state != STATE_MAIN)
    ResetTimer(TIMER_FSMMAIN_PWR_TIMER);
  
  switch(pwr_state)
  {
  case 0:
    if(!PSourseIsExternal())
    {
      ResetTimer(TIMER_FSMMAIN_PWR_TIMER);
      LED_Brightness(LED_BRIGHT_MAX / 2);
      pwr_state = 1;
    }
    else
    {
      if(SysParams.LEDBrVal == 0) //если режим Auto
      {
        IllumLevel_t tmp = LigthSensorVal();
        if(tmp == ILLUM_LIGHT) //если ярко
          LED_Brightness(LED_BRIGHT_MAX);
        else if(tmp == ILLUM_MEDIUM) //сумерки
          LED_Brightness(LED_BRIGHT_MAX / 3);
        else //темно
          LED_Brightness(LED_BRIGHT_MIN);
      }
      else
      {
        LED_Brightness(SysParams.LEDBrVal);
      }
    }
    break;
    /**********************************/
  case 1:
    if(GetTimer(TIMER_FSMMAIN_PWR_TIMER) >= TMR_SEC * 5)
    {
      LED_Off();
      pwr_state = 2;
    }
    else if(PSourseIsExternal())
    {
      pwr_state = 0;
    }
    else if(button != 0)
    {
      ResetTimer(TIMER_FSMMAIN_PWR_TIMER);
    }
    break;
    /**********************************/
  case 2:
    if(PSourseIsExternal())
    {
      LED_On();
      pwr_state = 0;
    }
    else if(button != 0)
    {
      button = 0;
      LED_On();
      ResetTimer(TIMER_FSMMAIN_PWR_TIMER);
      pwr_state = 1;
    }
    break;
    /**********************************/
  default:
    pwr_state = 0;
    break;
  }
  
  /************************************************/
  /* Тут обновляем значение текущего времени */  
  
  if(RTC_IsUpdate())
  {
    RTC_GetTime(&Time);
    flag_sep = 1;
    sec_cntr++;
    RTCCorrTry(&Time); //Функция корректировки времени
    ResetTimer(TIMER_FSMMAIN_SEP_TOUT);
  }
      
  if(GetTimer(TIMER_FSMMAIN_SEP_TOUT) >= TMR_SEC / 2)
  {
    flag_sep = 0;
  }
  /************************************************/
  
  /************************************************/
  /*********** Главный конечный автомат ***********/
  /************************************************/
  switch(state)
  {
  /************************************************/
  /************************************************/
  case STATE_MAIN: //Главный экран
    if(entry)
    {
      LED_BlinkOff(LED_BLINK_ALL);
      ButtonArptOffAll();
      RTC_GetTime(&Time);
      
      //flag_common = 0;
      sec_cntr = 0;
      
      PointsClear();
      
      //Сбрасываем конечный автомат главного экрана
      state_main(0, 0, 0, 1);
    }
    
    /**********************************/

    
    //Тут речевой информатор, 
    //который срабатывает каждый час
    AutoInfoTry(&Time);
    
    
    /**********************************/
    //Проверяем, 
    //не сработал ли будильник
    AlarmTry();
    
    /**********************************/
    
    //Конечный автомат главного экрана
    state_main(&sec_cntr, &Time, flag_sep, 0);
    
    //Обработчик нажатий на кнопки
    if(button == BTN_MENU)
    {
      state = STATE_MEINU_BEGIN;
    }
    else if(button == BTN_PLUS)
    {
      state = STATE_DISP_TEMPER_OUTDOOR;
      //INCR(br, LED_BRIGHT_MAX);
      //LED_Brightness(br);
    }
    else if(button == BTN_MINUS)
    {
      //DECR(br, LED_BRIGHT_MIN);
      //LED_Brightness(br);  
    }
    else if(button == BTN_TELL)
    {
      InfoExe(&Time, SysParams.ConfigInform);
      //Time2Voice(Time.RTC_Hours, Time.RTC_Minutes);
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_ALARM;
      /*
      //TestPlay(42 + 2);
      //VoiceAbort();
      if(VoiseIsReady())
        AlarmPlay(2, 2);
      else
        VoiceAbort();
      //Time2Voice(Time.RTC_Hours, Time.RTC_Minutes);
      */
    }
    break;
    
/************************************************/
  case STATE_DISP_TEMPER_OUTDOOR:
    if(entry)
    {
      sec_cntr = 0;
    }
    
    if(GetTemperStatus(OUTDOOR_SENSOR) != -1)
      DisplayTemper(GetTemper(OUTDOOR_SENSOR), 0);   
    else
      state = STATE_DISP_TEMPER_INDOOR;
    
    if(sec_cntr >= 3)
      state = STATE_MAIN;
    
    if(button == BTN_PLUS)
    {
      state = STATE_DISP_TEMPER_INDOOR;
    }
    else if(button == BTN_ALARM || button == BTN_MENU)
    {
      state = STATE_MAIN;
    }
    
    
    break;
/************************************************/
  case STATE_DISP_TEMPER_INDOOR:
    if(entry)
    {
      sec_cntr = 0;
    }
    
    if(GetTemperStatus(INDOOR_SENSOR) != -1)
      DisplayTemper(GetTemper(INDOOR_SENSOR), 1);   
    else
      state = STATE_MAIN;
    
    if(sec_cntr >= 3)
      state = STATE_MAIN;
    
    if(button == BTN_ALARM 
       || button == BTN_MENU
       || button == BTN_PLUS)
    {
      state = STATE_MAIN;
    }
    
    break;
/************************************************/
/************** Меню будильника *****************/
/************************************************/
  case STATE_ALARM:
    if(AlarmIsOn()) //Если будильник включен
    {
      //то отключаем его и выходим
      AlarmOff();
      state = STATE_MENU_EXIT;
    }
    else
    {
      //иначе включаем его
      //переходим в состояние показа будильника
      AlarmOn();
      state = STATE_ALARM_DISP;
    }
    
    SysParams.AlarmIsEnable = AlarmIsOn();
    ParamsSave(&SysParams);
    break;
    
/************************************************/    
  case STATE_ALARM_DISP:
    if(entry)
    {
      AlarmGet(&TmpTime);
      ResetTimer(TIMER_FSMMAIN_GENERAL);
    }
    DisplayAlarm(&TmpTime); //Отображаем значение будильника
    
    //Отображаем значение будильника 2 секунды
    if(GetTimer(TIMER_FSMMAIN_GENERAL) >= TMR_SEC * 2)
      state = STATE_MENU_EXIT;
    
    //Обработчик нажатий на кнопки
    if(button == BTN_MENU || button == BTN_ALARM) //Нажали Menu или Alarm
    {
      state = STATE_ALARM_SETHH;
    }
    break;
    
/************************************************/    
  case STATE_ALARM_SETHH:
    if(entry)
    {
      FlickerAlarmHH();
      gflag = 0;
      
      //Включаем автоповтор для + и -
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
    }
    DisplayAlarm(&TmpTime); //Отображаем значение будильника
    
    //Обработчик нажатий на кнопки
    if(button == BTN_MENU || button == BTN_ALARM) //Нажали Menu или Alarm
    {
      state = STATE_ALARM_SETMM;
    }
    else if(button == BTN_PLUS)
    {
      IncrUInt8(&TmpTime.RTC_Hours, 0, 23);
      gflag = 1;
      LED_BlinkTimerReset();
    }
    else if(button == BTN_MINUS)
    {
      DecrUInt8(&TmpTime.RTC_Hours, 0, 23);
      gflag = 1;
      LED_BlinkTimerReset();
    }
    
    break;
    
/************************************************/    
  case STATE_ALARM_SETMM:
    if(entry)
    {
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
      FlickerAlarmMM();
    }
    DisplayAlarm(&TmpTime); //Отображаем значение будильника
    
    //Обработчик нажатий на кнопки
    if(button == BTN_MENU || button == BTN_ALARM) //Нажали Menu или Alarm
    {
      if(gflag)
      {
        AlarmSet(&TmpTime);
        SysParams.AlarmHH = TmpTime.RTC_Hours;
        SysParams.AlarmMM = TmpTime.RTC_Minutes;
        SysParams.AlarmIsEnable = AlarmIsOn();
        ParamsSave(&SysParams);
      }
      state = STATE_MENU_EXIT;
    }
    else if(button == BTN_PLUS)
    {
      IncrUInt8(&TmpTime.RTC_Minutes, 0, 59);
      gflag = 1;
      LED_BlinkTimerReset();
    }
    else if(button == BTN_MINUS)
    {
      DecrUInt8(&TmpTime.RTC_Minutes, 0, 59);
      gflag = 1;
      LED_BlinkTimerReset();
    }
    break;
    
    
/************************************************/
/************************************************/
  case STATE_MEINU_BEGIN: //точка входа в меню
    if(entry)
    {
    }
    state = STATE_SETUP_HOUR;
    break;
    
/************************************************/
/********** Настройка текущего времени  *********/
/************************************************/
  case STATE_SETUP_HOUR:
    if(entry)
    {
      RTC_GetTime(&TmpTime);
      gflag = 0;
      
      //LED_BlinkTimerReset();
      LED_BlinkOn(LED_BLINK_0);
      LED_BlinkOn(LED_BLINK_1);
      
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
      
      PointsTimeSetup();
    }
    
    DisplayTime(&TmpTime, 1);
    
    if(button == BTN_PLUS)
    {
      gflag = 1;
      IncrUInt8(&TmpTime.RTC_Hours, 0, 23);
      LED_BlinkTimerReset();
    }
    else if(button == BTN_MINUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      DecrUInt8(&TmpTime.RTC_Hours, 0, 23);
    }
    else if(button == BTN_MENU)
    {
      state = STATE_SETUP_MIN;
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_MENU_EXIT;
    }
    break;

/************************************************/
  case STATE_SETUP_MIN:
    if(entry)
    {
      LED_BlinkOffAll();
      
      LED_BlinkOn(LED_BLINK_3);
      LED_BlinkOn(LED_BLINK_4);
      
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
    }
    
    DisplayTime(&TmpTime, 1);
    
    //обработка нажатий на кнопки
    if(button == BTN_PLUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      IncrUInt8(&TmpTime.RTC_Minutes, 0, 59);
    }
    else if(button == BTN_MINUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      DecrUInt8(&TmpTime.RTC_Minutes, 0, 59);
    }
    else if(button == BTN_MENU)
    {
      //сохраняем время
      if(gflag)
      {
        TmpTime.RTC_Seconds = 0;
        RTC_SetTime(&TmpTime);
      }
      
      //переходим далее
      state = STATE_SETUP_INFO;
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_MENU_EXIT;
    }
    
    
    
    break;
    
/************************************************/
/************************************************/
  case STATE_SETUP_INFO:
    if(entry)
    {
      PointsVInfo();
      FlickerVInfoStatus();
      ButtonArptOffAll();
      gflag = 0;
      UInt8Tmp = SysParams.VoiseInfoEnable;
    }
    DisplayVInfo(SysParams.VoiseInfoBegin,
                 SysParams.VoiseInfoEnd,
                 UInt8Tmp);
    
    //обработка нажатий на кнопки
    if(button == BTN_PLUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      IncrUInt8(&UInt8Tmp, 0, 1);
    }
    else if(button == BTN_MINUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      DecrUInt8(&UInt8Tmp, 0, 1);
    }
    else if(button == BTN_MENU)
    {
      if(gflag)
      {
        SysParams.VoiseInfoEnable = UInt8Tmp;
        ParamsSave(&SysParams);
      }
      
      //переходим далее
      if(SysParams.VoiseInfoEnable)
        state = STATE_SETUP_INFO_BEGIN;
      else
        state = STATE_SETUP_ALARM_MELODY;
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_MENU_EXIT;
    }
    break;
    
/************************************************/    
  case STATE_SETUP_INFO_BEGIN:
    if(entry)
    {
      FlickerVInfoBegin();
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
      gflag = 0;
      UInt8Tmp = SysParams.VoiseInfoBegin;
    }
    
    DisplayVInfo(UInt8Tmp,
                 SysParams.VoiseInfoEnd,
                 SysParams.VoiseInfoEnable);
    
    //обработка нажатий на кнопки
    if(button == BTN_PLUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      IncrUInt8(&UInt8Tmp, 0, 23);
    }
    else if(button == BTN_MINUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      DecrUInt8(&UInt8Tmp, 0, 23);
    }
    else if(button == BTN_MENU)
    {
      if(gflag)
      {
        SysParams.VoiseInfoBegin = UInt8Tmp;
        ParamsSave(&SysParams);
      }
      //переходим далее
      state = STATE_SETUP_INFO_END;
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_MENU_EXIT;
    }
    
    break;
/************************************************/    
  case STATE_SETUP_INFO_END:
    if(entry)
    {
      FlickerVInfoEnd();
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
      gflag = 0;
      UInt8Tmp = SysParams.VoiseInfoEnd;
    }
    
    DisplayVInfo(SysParams.VoiseInfoBegin,
                 UInt8Tmp,
                 SysParams.VoiseInfoEnable);
    
    //обработка нажатий на кнопки
    if(button == BTN_PLUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      IncrUInt8(&UInt8Tmp, 0, 23);
    }
    else if(button == BTN_MINUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      DecrUInt8(&UInt8Tmp, 0, 23);
    }
    else if(button == BTN_MENU)
    {
      if(gflag)
      {
        SysParams.VoiseInfoEnd = UInt8Tmp;
        ParamsSave(&SysParams);
      }
      //переходим далее
      state = STATE_SETUP_ALARM_MELODY;
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_MENU_EXIT;
    }
    
    break;
/************************************************/
/************************************************/
  case STATE_SETUP_ALARM_MELODY:
    if(entry)
    {
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
      FlickerAlarmMelody();
      PointsAlarmMelody();
      
      gflag = 0;
      UInt8Tmp = SysParams.AlarmMelody;
    }
    DisplayAlarmMelody(UInt8Tmp);
    
    //если играет мелодия
    //то сбрасываем таймер выхода по таймауту
    if(!VoiseIsReady())
      ResetTimer(TIMER_FSMMAIN_TIMEOUT);
    
    //обработка нажатий на кнопки
    if(button == BTN_PLUS)
    {
      //если что-то играет, то глушим
      VoiceAbort();
      
      gflag = 1;
      LED_BlinkTimerReset();
      IncrUInt8(&UInt8Tmp, 0, GetNumAlarms() - 1);
    }
    else if(button == BTN_MINUS)
    {
      //если что-то играет, то глушим
      VoiceAbort();
      
      gflag = 1;
      LED_BlinkTimerReset();
      DecrUInt8(&UInt8Tmp, 0, GetNumAlarms() - 1);
    }
    else if(button == BTN_MENU)
    {
      if(gflag)
      {
        SysParams.AlarmMelody = UInt8Tmp;
        ParamsSave(&SysParams);
      }
      
      //если что-то играет, то глушим
      VoiceAbort();
      
      //переходим далее
      state = STATE_SETUP_BRIGHT;
    }
    else if(button == BTN_ALARM)
    {
      VoiceAbort();
      state = STATE_MENU_EXIT;
    }
    else if(button == BTN_TELL)
    {
      //включаем выбранную мелодию
      if(VoiseIsReady())
        AlarmPlay(UInt8Tmp, 1);
      else
        VoiceAbort();
    }
    break;
/************************************************/
/************************************************/
  case STATE_SETUP_BRIGHT:
    if(entry)
    {
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
      FlickerBrVal();
      PointsBrVal();
      
      gflag = 0;
      UInt8Tmp = SysParams.LEDBrVal;
    }
    
    DisplayBrVal(UInt8Tmp);
    
    //обработка нажатий на кнопки
    if(button == BTN_PLUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      UncircIncrUInt8(&UInt8Tmp, LED_BRIGHT_MAX);
    }
    else if(button == BTN_MINUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      UncircDecrUInt8(&UInt8Tmp, LED_BRIGHT_MIN);
    }
    else if(button == BTN_MENU)
    {
      if(gflag)
      {
        SysParams.LEDBrVal = UInt8Tmp;
        ParamsSave(&SysParams);
      }
      //переходим далее
      state = STATE_SETUP_RTC_CORR;
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_MENU_EXIT;
    }
    
    break;
    
/************************************************/
/************************************************/
  case STATE_SETUP_RTC_CORR:
    if(entry)
    {
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
      PointsRTCCorr();
      FlickerRTCCorr();
      gflag = 0;
      Int8Tmp = SysParams.ClockCorrCnsnt;
    }
    
    DisplayRTCCorr(Int8Tmp);
    
    //обработка нажатий на кнопки
    if(button == BTN_PLUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      UncircIncrInt8(&Int8Tmp, 25);
    }
    else if(button == BTN_MINUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      UncircDecrInt8(&Int8Tmp, -25);
    }
    else if(button == BTN_MENU)
    {
      if(gflag)
      {
        SysParams.ClockCorrCnsnt = Int8Tmp;
        ParamsSave(&SysParams);
      }
      //переходим далее
      state = STATE_SETUP_CONFIG;
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_MENU_EXIT;
    }
    break;
/************************************************/
/************************************************/
  case STATE_SETUP_CONFIG:
    state = STATE_SETUP_CONFIG_DISPLAY;
    break;
/************************************************/   
  case STATE_SETUP_CONFIG_DISPLAY:
    if(entry)
    {
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
      PointsConfig();
      FlickerConfigDisplay();
      gflag = 0;
      UInt8Tmp = SysParams.ConfigDisplay;
    }
    
    DisplayConfigDisplay(UInt8Tmp, 
                         SysParams.ConfigInform, 
                         SysParams.ConfigAutoInform, 
                         0);
    
    //обработка нажатий на кнопки
    if(button == BTN_PLUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      UncircIncrUInt8(&UInt8Tmp, 3); //4 ватианта дисплея: 0, 1, 2, 3
    }
    else if(button == BTN_MINUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      UncircDecrUInt8(&UInt8Tmp, 0);
    }
    else if(button == BTN_MENU)
    {
      if(gflag)
      {
        SysParams.ConfigDisplay = UInt8Tmp;
        ParamsSave(&SysParams);
      }
      //переходим далее
      state = STATE_SETUP_CONFIG_INFORM;
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_MENU_EXIT;
    }
    
    break;
/************************************************/   
  case STATE_SETUP_CONFIG_INFORM:
    if(entry)
    {
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
      PointsConfig();
      FlickerConfigInform();
      gflag = 0;
      UInt8Tmp = SysParams.ConfigInform;
    }
    
    DisplayConfigDisplay(SysParams.ConfigDisplay, 
                         UInt8Tmp, 
                         SysParams.ConfigAutoInform, 
                         1);
    
    //обработка нажатий на кнопки
    if(button == BTN_PLUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      UncircIncrUInt8(&UInt8Tmp, 2); //3 ватианта информатора: 0, 1, 2
    }
    else if(button == BTN_MINUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      UncircDecrUInt8(&UInt8Tmp, 0);
    }
    else if(button == BTN_MENU)
    {
      if(gflag)
      {
        SysParams.ConfigInform = UInt8Tmp;
        ParamsSave(&SysParams);
      }
      //переходим далее
      state = STATE_SETUP_CONFIG_AUTOINFORM;
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_MENU_EXIT;
    }
    break;
/************************************************/   
  case STATE_SETUP_CONFIG_AUTOINFORM:
    if(entry)
    {
      ButtonArptOffAll();
      ButtonArptOn(BTN_PLUS);
      ButtonArptOn(BTN_MINUS);
      PointsConfig();
      FlickerConfigAutoInform();
      gflag = 0;
      UInt8Tmp = SysParams.ConfigAutoInform;
    }
    
    DisplayConfigDisplay(SysParams.ConfigDisplay, 
                         SysParams.ConfigInform, 
                         UInt8Tmp, 
                         2);
    
    
    //обработка нажатий на кнопки
    if(button == BTN_PLUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      UncircIncrUInt8(&UInt8Tmp, 2); //3 ватианта АвтоИнформатора: 0, 1, 2
    }
    else if(button == BTN_MINUS)
    {
      gflag = 1;
      LED_BlinkTimerReset();
      UncircDecrUInt8(&UInt8Tmp, 0);
    }
    else if(button == BTN_MENU)
    {
      if(gflag)
      {
        SysParams.ConfigAutoInform = UInt8Tmp;
        ParamsSave(&SysParams);
      }
      //переходим далее
      state = STATE_MENU_EXIT;
    }
    else if(button == BTN_ALARM)
    {
      state = STATE_MENU_EXIT;
    }
    break;
/************************************************/
/*********** Точка выхода из меню ***************/
/************************************************/
  case STATE_MENU_EXIT:
    
    state = STATE_MAIN;
    break;
    
/************************************************/
  default:
    state = STATE_MAIN;
    break;
  }
  
  
  
  
  
}