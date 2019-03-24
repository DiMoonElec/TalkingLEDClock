#ifndef __TEMPER_H__
#define __TEMPER_H__

#include <stdint.h>

#define INDOOR_SENSOR    0
#define OUTDOOR_SENSOR   1

void InitTemper(void);
void ProcessTemper(void);
float GetTemper(int n);
int GetTemperStatus(int n);

#endif

