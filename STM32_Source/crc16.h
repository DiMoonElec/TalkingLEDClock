#ifndef __CRC16_H__
#define __CRC16_H__

#include <stdint.h>
/*
void Crc16Reset(void);
uint16_t Crc16Get(void);
uint16_t Crc16(uint8_t data);
*/
unsigned short Crc16(unsigned char * pcBlock, unsigned short len);

#endif
