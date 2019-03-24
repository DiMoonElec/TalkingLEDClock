#include <stdint.h>
#include "media.h"
#include "voice.h"
#include "voice_table.h"
#include "at45db321.h"
#include "timers.h"

/********************************************************************/

#define DF_READ(buff, address, len)    at45_Read((buff), (address), (len))

/********************************************************************/

typedef struct
{
  uint32_t start;
  uint32_t lenght;
} SampleData_t;

//static uint32_t byte_counter;
//static uint32_t data_len;
//static uint8_t flag_playcomp;

static SampleData_t Chunk[12];
static SampleData_t SampleDataFlash[56];
static uint8_t NumAlarms = 0;

/********************************************************************/

#define H_IS_NVOID_SAMPLE(hh, n)     (hours_table[hh][n] != SAMPLE_void)
#define M_IS_NVOID_SAMPLE(mm, n)     (min_table[mm][n] != SAMPLE_void)

#define TEMPER_IS_VOID_SAMPLE(tt, n)     (temper_table[tt][n] == SAMPLE_void)

#define H_GET_SAMPLE(hh, n)     SampleDataFlash[hours_table[hh][n]]
#define M_GET_SAMPLE(mm, n)     SampleDataFlash[min_table[mm][n]]

#define TEMPER_GET_SAMPLE(tt, n)     SampleDataFlash[temper_table[tt][n]]

/********************************************************************/

#define CHANNEL_INFO    0
#define CHANNEL_ALARM   1

static uint8_t NumChunks;

enum {
  ST_READY = 0, 
  ST_RESTART,
  ST_RUN,
  ST_RUN_DELAY,
  ST_CIRC_PLAY,
  ST_CIRC_WAIT,
  ST_PLAY,
  ST_WAIT
};

static uint8_t state;

static uint8_t flag_run = 0;
static uint8_t flag_circ = 0;
static uint8_t flag_last = 0;
static uint8_t flag_abort = 0;
static uint8_t flag_restart = 0;
static uint8_t channel = 0;
static uint32_t num_circ = 0;

/********************************************************************/

static uint8_t AlarmsCount(void)
{
  uint8_t tmp = 0;
  
  for(int i=0; i<16; i++)
  {
    if(SampleDataFlash[SAMPLE_alarm1 + i].start == 0)
      break;
    tmp++;
  }
  
  return tmp;
}

/********************************************************************/

void VoiceInit(void)
{
  media_init();
  DF_READ(SampleDataFlash, 0, sizeof(SampleDataFlash));
  NumAlarms = AlarmsCount();
}

uint8_t GetNumAlarms(void)
{
  return NumAlarms;
}

uint32_t GetAlarmLen(uint8_t an)
{
  if(an >= NumAlarms)
    return 0;
  
  return SampleDataFlash[SAMPLE_alarm1 + an].lenght;
}

uint8_t VoiseIsReady(void)
{
  return (state == ST_READY);
}


void TimeTemper2Voice(uint8_t hh, uint8_t mm, uint8_t temper, uint8_t flag_minus, 
                      uint8_t flag_IsOutTemper)
{
  uint8_t c = 0;
  uint8_t i;
  
  if(state != ST_READY)
    return;
  
  Chunk[c].start = SampleDataFlash[SAMPLE_bell].start;
  Chunk[c].lenght = SampleDataFlash[SAMPLE_bell].lenght;
  c++;
  
  for(i=0; i<3; i++)
  {
    if(H_IS_NVOID_SAMPLE(hh, i))
    {
      Chunk[c].start = H_GET_SAMPLE(hh, i).start;
      Chunk[c].lenght = H_GET_SAMPLE(hh, i).lenght;
      c++;
    }
  }
  
  for(i=0; i<3; i++)
  {
    if(M_IS_NVOID_SAMPLE(mm, i))
    {
      Chunk[c].start = M_GET_SAMPLE(mm, i).start;
      Chunk[c].lenght = M_GET_SAMPLE(mm, i).lenght;
      c++;
    }
  }
  
  if(temper < 60)
  {
    if(flag_IsOutTemper)
    {
      Chunk[c].start = SampleDataFlash[SAMPLE_temperatura_out].start;
      Chunk[c].lenght = SampleDataFlash[SAMPLE_temperatura_out].lenght;
    }
    else
    {
      Chunk[c].start = SampleDataFlash[SAMPLE_temperatura_in].start;
      Chunk[c].lenght = SampleDataFlash[SAMPLE_temperatura_in].lenght;
    }
    c++;
    
    if(flag_minus && temper != 0)
    {
      Chunk[c].start = SampleDataFlash[SAMPLE_minus].start;
      Chunk[c].lenght = SampleDataFlash[SAMPLE_minus].lenght;
      c++;
    }
    
    for(i=0; i<3; i++)
    {
      if(TEMPER_IS_VOID_SAMPLE(temper, i))
        break;
      
      Chunk[c].start = TEMPER_GET_SAMPLE(temper, i).start;
      Chunk[c].lenght = TEMPER_GET_SAMPLE(temper, i).lenght;
      c++;
    }
  }
  NumChunks = c;
  
  flag_circ = 0;
  flag_last = 0;
  flag_abort = 0;
  channel = CHANNEL_INFO;
  
  flag_run = 1;
}

void Time2Voice(uint8_t hh, uint8_t mm)
{
  uint8_t c = 0;
  uint8_t i;
  
  if(state != ST_READY)
    return;
  
  Chunk[c].start = SampleDataFlash[SAMPLE_bell].start;
  Chunk[c].lenght = SampleDataFlash[SAMPLE_bell].lenght;
  c++;
  
  for(i=0; i<3; i++)
  {
    if(H_IS_NVOID_SAMPLE(hh, i))
    {
      Chunk[c].start = H_GET_SAMPLE(hh, i).start;
      Chunk[c].lenght = H_GET_SAMPLE(hh, i).lenght;
      c++;
    }
  }
  
  for(i=0; i<3; i++)
  {
    if(M_IS_NVOID_SAMPLE(mm, i))
    {
      Chunk[c].start = M_GET_SAMPLE(mm, i).start;
      Chunk[c].lenght = M_GET_SAMPLE(mm, i).lenght;
      c++;
    }
  }

  NumChunks = c;
  
  flag_circ = 0;
  flag_last = 0;
  flag_abort = 0;
  flag_restart = 0;
  channel = CHANNEL_INFO;
  
  flag_run = 1;
}

void VoiceAbort(void)
{
  if(state != ST_READY)
    flag_abort = 1;
}

void TestPlay(uint8_t sample)
{
  if(state != ST_READY)
    return;
  
  Chunk[0].start = SampleDataFlash[sample].start;
  Chunk[0].lenght = SampleDataFlash[sample].lenght;
  
  NumChunks = 1;
  
  flag_circ = 0;
  flag_last = 0;
  flag_abort = 0;
  channel = CHANNEL_INFO;
  
  flag_run = 1;
}

void AlarmPlay(uint8_t an, uint32_t cycles)
{
  if(state != ST_READY)
    flag_restart = 1;
  else 
    flag_restart = 0;
    
  flag_abort = 0;
  
  if(an >= NumAlarms)
    an = 0;
  
  Chunk[0].start = SampleDataFlash[SAMPLE_alarm1 + an].start;
  Chunk[0].lenght = SampleDataFlash[SAMPLE_alarm1 + an].lenght;
  
  num_circ = cycles;
  channel = CHANNEL_INFO;
  flag_circ = 1;
  flag_run = 1;
}

void VoiceProcess(void)
{
  static uint8_t i = 0;
  
  switch(state)
  {
    /*******************************************/
  case ST_READY:
    if(flag_run)
    {
      flag_run = 0;
      state = ST_RUN;
    }
    break;
    
  case ST_RESTART:
    flag_run = 0;
    flag_restart = 0;
    state = ST_RUN;
    break;
    
    /*******************************************/
    /*******************************************/
  case ST_RUN:
    if(channel == CHANNEL_INFO)
      media_enable(CHANNEL_A);
    else
      media_enable(CHANNEL_B);
    
    ResetTimer(TIMER_VOICE_TIMEOUT);
    state = ST_RUN_DELAY;
    
    break;
    
    /*******************************************/
  case ST_RUN_DELAY:
    if(GetTimer(TIMER_VOICE_TIMEOUT) >= TMR_SEC / 2)
    {
      i = 0;    
      if(flag_circ)
        state = ST_CIRC_PLAY;
      else
        state = ST_PLAY;
    }
    
    break;
    
    /*******************************************/
    /*******************************************/
  case ST_CIRC_PLAY:
    if(i < num_circ)
    {
      media_play(Chunk[0].start, Chunk[0].lenght);
      i++;
      state = ST_CIRC_WAIT;
    }
    else
    {
      media_disable();
      state = ST_READY;
    }
    break;
    
    /*******************************************/
  case ST_CIRC_WAIT:
    if(flag_abort)
    {
      flag_abort = 0;
      media_abort();
      media_disable();
      state = ST_READY;
    }
    else if(flag_restart)
    {
      flag_abort = 0;
      media_abort();
      media_disable();
      state = ST_RESTART;
    }
    else if(media_isrdy())
    {
      if(flag_last)
      {
        flag_last = 0;
        media_disable();
        state = ST_READY;
      }
      else
      {
        state = ST_CIRC_PLAY;
      }
    }
    break;
    
    /*******************************************/
    /*******************************************/
    
  case ST_PLAY:
    if(i<NumChunks)
    {
      media_play(Chunk[i].start, Chunk[i].lenght);
      i++;
      state = ST_WAIT;
    }
    else
    {
      media_disable();
      state = ST_READY;
    }
    break;
    
    /*******************************************/
  case ST_WAIT:
    if(flag_abort)
    {
      flag_abort = 0;
      media_abort();
      media_disable();
      state = ST_READY;
    }
    else if(flag_restart)
    {
      flag_restart = 0;
      media_abort();
      media_disable();
      state = ST_RESTART;
    }
    else if(media_isrdy())
    {
      state = ST_PLAY;
    }
    
    break;
    
    /*******************************************/
    /*******************************************/
  default:
    flag_run = 0;
    state = ST_READY;
    break;
  }
  
#if 0
  static uint8_t i = 0;
  
  switch(state)
  {
  case ST_READY:
    break;
    
  case ST_PLAY:
    i = 0;
    state = ST_PLAY_1;
    /*
    PCM_OUT(127);
    AMP_ENABLE();
    */
    media_enable(CHANNEL_A);
    break;
    
  case ST_PLAY_1:
    if(i<NumChunks)
    {
      media_play(Chunk[i].start, Chunk[i].lenght);
      i++;
      state = ST_PLAY_2;
    }
    else
    {
      /*
      AMP_DISABLE();
      PCM_OUT(0x00);
      */
      media_disable();
      state = ST_READY;
    }
    break;
    
  case ST_PLAY_2:
    if(media_isrdy())
      state = ST_PLAY_1;
    break;
  }
  
#endif
  
}
