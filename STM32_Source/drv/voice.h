#ifndef __VOICE_H__
#define __VOICE_H__

#include <stdint.h>

void VoiceInit(void);
uint8_t VoiseIsReady(void);
void TimeTemper2Voice(uint8_t hh, uint8_t mm, uint8_t temper, uint8_t flag_minus, 
                      uint8_t flag_IsOutTemper);
void Time2Voice(uint8_t hh, uint8_t mm);
void VoiceProcess(void);

void TestPlay(uint8_t sample);
void VoiceAbort(void);

uint8_t GetNumAlarms(void);
uint32_t GetAlarmLen(uint8_t an);
void AlarmPlay(uint8_t an, uint32_t cycles);

#define SAMPLE_RATE     22050

#endif
