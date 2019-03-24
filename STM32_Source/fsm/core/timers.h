#ifndef __TIMERS_H__
#define __TIMERS_H__

enum
{
  TIMER_TEMPER_GENERAL = 0,
  TIMER_FSMMAIN_GENERAL,
  TIMER_FSMMAIN_TIMEOUT,
  TIMER_FSMMAIN_SEP_TOUT,
  TIMER_FSMMAIN_PWR_TIMER,
  TIMER_LIGHT_SENSOR,
  TIMER_VOICE_TIMEOUT,
  NUM_TIMERS
};

#define MAX_TIMERS      NUM_TIMERS

#define TMR_TICK_MS     10
#define TMR_SEC         100
#define TMR_MIN         (TMR_SEC * 60)
#define TMR_HOUR        (TMR_MIN * 60)

void InitTimers(void);
uint32_t GetTimer(uint32_t TimerID);
void ResetTimer(uint32_t TimerID);

#endif
