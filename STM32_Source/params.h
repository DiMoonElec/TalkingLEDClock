#ifndef __PARAMS_H__
#define __PARAMS_H__

#include <stdint.h>


/*
 !!! ¬Ќ»ћјЌ»≈!!!
 –азмер структуры SysParams_s 
 должен быть кратен 2-м
 и не больше 20 байт!!!

 !!! ATTENTION!!!
†The size of the SysParams_s structure 
 must be a multiple of 2 
 and not more than 20 bytes !!!
*/
#pragma pack(push, 1)

struct SysParams_s
{
  //–ечевой информатор
  uint8_t VoiseInfoBegin;
  uint8_t VoiseInfoEnd;
  uint8_t VoiseInfoEnable;
  
  //¤ркость индикатора
  uint8_t LEDBrVal;
  
  //Ѕудильнк
  uint8_t AlarmHH;
  uint8_t AlarmMM;
  uint8_t AlarmIsEnable;
  uint8_t AlarmMelody;
  
  //корректировка хода часов
  int8_t ClockCorrCnsnt;
  
  uint8_t ConfigDisplay; //¬ариант главного экрана
  uint8_t ConfigInform;  //¬ариант информатора при нажатии на кнопку
  uint8_t ConfigAutoInform; //¬ариант автоинформатора, срабатывающего каждый час
  //uint8_t reserved
};

#pragma pack(pop)


void ParamsLoad(struct SysParams_s *params);
void ParamsSave(struct SysParams_s *params);


#endif
