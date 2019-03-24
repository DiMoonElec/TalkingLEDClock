#include <stdint.h>
#include "signals.h"


static char Signals[MAX_SIGNALS];
static char BroadcastSignals[MAX_BROADCAST_SIGNALS];

void InitSignals(void)
{
  char i;
  for(i = 0; i < MAX_SIGNALS; i++)
    Signals[i] = 0;
  for(i = 0; i < MAX_BROADCAST_SIGNALS; i++)
    BroadcastSignals[i] = 0;
}

void SendSignal(char Sgnl)
{
  if(Signals[Sgnl] == 0)
    Signals[Sgnl] = 1;
}

void SendBroadcastSignal(char Sgnl)
{
  if(BroadcastSignals[Sgnl] == 0)
    BroadcastSignals[Sgnl] = 1;
}

void ProcessSignals(void)
{
  char i;
  for(i = 0; i < MAX_SIGNALS; i++)
  {
    if(Signals[i] == 2) Signals[i] = 0;
    if(Signals[i] == 1) Signals[i] = 2;
  }
  for(i = 0; i < MAX_BROADCAST_SIGNALS; i++)
  {
    if(BroadcastSignals[i] == 2) BroadcastSignals[i] = 0;
    if(BroadcastSignals[i] == 1) BroadcastSignals[i] = 2;
  }
}

char GetSignal(char Sgnl)
{
  if(Signals[Sgnl] == 2)
  {
    Signals[Sgnl] = 0;
    return 1;
  }
  return 0;
}

char GetBroadcastSignal(char Sgnl)
{
  if(Signals[Sgnl] == 2)
    return 1;
  return 0;
}



