#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#define MAX_SIGNALS             1
#define MAX_BROADCAST_SIGNALS   1

void InitSignals(void);
void ProcessSignals(void);
void SendSignal(char Sgnl);
void SendBroadcastSignal(char Sgnl);
char GetSignal(char Sgnl);
char GetBroadcastSignal(char Sgnl);

#endif