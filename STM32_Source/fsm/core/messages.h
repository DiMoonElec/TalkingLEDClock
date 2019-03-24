#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#define MAX_MESSAGES            1
#define MAX_BROADCAST_MESSAGES  1

void InitMessages(void);
void ProcessMessages(void);

#if MAX_MESSAGES > 0
void SendMessage(char Msg, void* Param);
void* GetMessage(char Msg);
#endif

#if MAX_BROADCAST_MESSAGES > 0
void SendBroadcastMessage(char Msg, void* Param);
void* GetBroadcastMessage(char Msg);
#endif

#endif