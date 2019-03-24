#include <stdint.h>
#include "messages.h"

typedef struct
{
  char Status;
  void *ParamPtr;
} MSG_DATA;


#if MAX_MESSAGES > 0
static MSG_DATA Messages[MAX_MESSAGES];
#endif

#if MAX_BROADCAST_MESSAGES > 0
static MSG_DATA BroadcastMessages[MAX_BROADCAST_MESSAGES];
#endif

void InitMessages(void)
{
  char i;
#if MAX_MESSAGES > 0
  for(i = 0; i < MAX_MESSAGES; i++)
  {
    Messages[i].Status = 0;
    Messages[i].ParamPtr = 0;
  }
#endif
  
#if MAX_BROADCAST_MESSAGES > 0
  for(i = 0; i < MAX_BROADCAST_MESSAGES; i++)
  {
    BroadcastMessages[i].Status = 0;
    BroadcastMessages[i].ParamPtr = 0;
  }
#endif
}


void ProcessMessages(void)
{
  char i;
#if MAX_MESSAGES > 0
  for(i = 0; i < MAX_MESSAGES; i++)
  {
    if(Messages[i].Status == 2) Messages[i].Status = 0;
    if(Messages[i].Status == 1) Messages[i].Status = 2;
  }
#endif
  
#if MAX_BROADCAST_MESSAGES > 0
  for(i = 0; i < MAX_BROADCAST_MESSAGES; i++)
  {
    if(BroadcastMessages[i].Status == 2) BroadcastMessages[i].Status = 0;
    if(BroadcastMessages[i].Status == 1) BroadcastMessages[i].Status = 2;
  }
#endif
}

#if MAX_MESSAGES > 0
void* GetMessage(char Msg)
{
  if(Messages[Msg].Status == 2)
  {
    Messages[Msg].Status = 0;
    return Messages[Msg].ParamPtr;
  }
  return 0;
}

void SendMessage(char Msg, void* Param)
{
  if(Messages[Msg].Status == 0)
  {
    Messages[Msg].Status = 1;
    Messages[Msg].ParamPtr = Param;
  }
}

#endif

#if MAX_BROADCAST_MESSAGES > 0
void* GetBroadcastMessage(char Msg)
{
  if(BroadcastMessages[Msg].Status == 2)
    return BroadcastMessages[Msg].ParamPtr;
  return 0;
}

void SendBroadcastMessage(char Msg, void* Param)
{
  if(BroadcastMessages[Msg].Status == 0)
  {
    BroadcastMessages[Msg].Status = 1;
    BroadcastMessages[Msg].ParamPtr = Param;
  }
}
#endif