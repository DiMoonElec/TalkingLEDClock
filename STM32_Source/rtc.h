#ifndef __RTC_H__
#define __RTC_H__




/*
typedef struct
{
    uint8_t RTC_Hours;
    uint8_t RTC_Minutes;
    uint8_t RTC_Seconds;
    uint8_t RTC_Date;
    uint8_t RTC_Wday;
    uint8_t RTC_Month;
    uint16_t RTC_Year;
} RTC_DateTimeTypeDef;
*/


typedef struct
{
    uint8_t RTC_Hours;
    uint8_t RTC_Minutes;
    uint8_t RTC_Seconds;
} RTCTime_t;


int RTC_Init(void);
int RTC_IsUpdate(void);
//void RTC_GetDateTime(RTC_DateTimeTypeDef* RTC_DateTimeStruct);

void RTC_GetTime(RTCTime_t* time);
void RTC_SetTime(RTCTime_t* time);

int Alarm_Triggered(void);

void AlarmSet(RTCTime_t* time);
void AlarmGet(RTCTime_t* time);
int AlarmIsOn(void);
void AlarmOn(void);
void AlarmOff(void);

void BackupWrite(uint8_t address, uint16_t val);
uint16_t BackupRead(uint8_t address);
int BackupIsInit(void);

#endif