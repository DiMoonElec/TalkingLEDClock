#ifndef __AT45DB321_H_
#define __AT45DB321_H_


#define AT45_CS_PORT   PORTB
#define AT45_CS_DDR    DDRB
#define AT45_CS_PIN    3

/*
  Eieoeaeecaoey iee?inoaiu flash-iaiyoe
  e iineaaiaaoaeuiiai ii?oa SPI
*/
void at45_init(void);

/*
  buff - iiia? aioo?aiiaai aooa?a, 1 eee 2
  *data - aaiiua aey caiene a aooa?, ?acia? 528 aaeo 
*/
void at45_BuffWrite(uint8_t buff, uint8_t *data);

/*
  buff - iiia? aioo?aiiaai aooa?a, 1 eee 2
  *data - aaiiua, ?acia? 528 aaeo 
*/
void at45_BuffRead(uint8_t buff, uint8_t *data);

/*
  ia?anueea aioo?aiiaai aooa?a a iniiaio? flash-iaiyou
  buff - iiia? aooa?a 1 eee 2
  page - iiia? no?aieou a iniiaiie iaiyoe
*/
void at45_BuffToMainMem(uint8_t buff, uint32_t page);

/*
  Ia?aneaou aaiiua ec iniiaiie iaiyoe ai aioo?aiiee aooa?
  buff - iiia? aooa?a 1 eee 2
  page - no?aieoa a iniiaiie iaiyoe
*/
void at45_MainMemToBuff(uint8_t buff, uint32_t page);

/*
  I?iaa?ea aioiaiinoe iaiyoe
*/
uint8_t at45_IsReady(void);

/*
  ?oaiea len aaeo, ia?eiay n aa?ana address  a aooa? *buff
*/
void at45_Read(void *buff, uint32_t address, uint16_t len);

/*
  Eieoeaeecaoey iioieiaiai ?oaiey n flash-iaiyoe
*/
void at45_StreamStart(uint32_t address);

/*
  Caaa?oaiea iioieiaiai ?oaiey
*/
void at45_StreamStop(void);

/*
  ?oaiea aaeoa aaiiuo ec flash-iaiyoe 
  a iioieiaii ?a?eia
*/
uint8_t at45_StreamRead(void);

//void at45_StreamDMARead(void *buff, uint16_t len, void (*CallBack)(void));

/*
void at45_init(void);
char at45_isrdy(void);
void at45_Read(void *buff, uint32_t address, uint16_t len);
*/
#endif
