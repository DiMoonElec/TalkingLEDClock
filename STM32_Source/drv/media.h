#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <stdint.h>

typedef enum
{
  CHANNEL_A = 0,
  CHANNEL_B = 1
} MediaChannel_t;

void media_init(void);
void media_play(uint32_t start, uint32_t len);
void media_enable(MediaChannel_t channel);
void media_disable(void);
int media_isrdy(void);
void media_abort(void);


#endif
