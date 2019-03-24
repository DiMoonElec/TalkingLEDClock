#include <stdint.h>
#include "rtc.h"
#include "params.h"


static struct SysParams_s DefVals = { 
  .VoiseInfoBegin = 7,
  .VoiseInfoEnd = 22,
  .VoiseInfoEnable = 1,
  
  .LEDBrVal = 0,
  
  .AlarmHH = 7,
  .AlarmMM = 0,
  .AlarmIsEnable = 0,
  .AlarmMelody = 0,
  
  .ClockCorrCnsnt = 0,
  
  .ConfigDisplay = 0,
  .ConfigInform = 0,
  .ConfigAutoInform = 0
};

void ParamsLoad(struct SysParams_s *params)
{
  if(sizeof(struct SysParams_s) > 20)
    return;
  
  uint16_t *pntr = (uint16_t *)params;
  
  /*********************************/
  
  if(!BackupIsInit())
  {
    uint16_t *def_pntr = (uint16_t *)(&DefVals);
    
    for(uint8_t i=0; 
        i<(sizeof(struct SysParams_s) / 2); 
        i++)
    {
      pntr[i] = def_pntr[i];
    }
    
    return;
  }
  
  /*********************************/
  
  for(uint8_t i=0; 
      i<(sizeof(struct SysParams_s) / 2); 
      i++)
  {
    pntr[i] = BackupRead(i);
  }

}

void ParamsSave(struct SysParams_s *params)
{
  if(sizeof(struct SysParams_s) > 20)
    return;
  
  uint16_t *pntr = (uint16_t *)params;
  
  for(uint8_t i=0; 
      i<(sizeof(struct SysParams_s) / 2); 
      i++)
  {
    BackupWrite(i, pntr[i]);
  }
}

