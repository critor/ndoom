#include "tools.h"

#define SCREEN_PIXELS	(SCREEN_WIDTH*SCREEN_HEIGHT)
#define SCREEN_SIZE		(SCREEN_PIXELS*2)
#define CONTRAST_MIN	0x60
#define CONTRAST_MAX	0xC0

#define rgb2gs(r,g,b)	(((~((((r)*30+(g)*59+(b)*11)/100)>>4))&0b1111)<<1)
#define rgb565(r,g,b)	((((r)<<8)&0xf800)|(((g)<<3)&0x07e0)|((b)>>3))

unsigned int getContrast();
void drwBufStr(uint16_t* buf, uint16_t x, uint16_t y, char* str, uint8_t ret, uint8_t trsp);
void putBufChar(uint16_t* buf, uint16_t x, uint16_t y, char ch, uint8_t trsp);
void setBufPixel(uint16_t* buf, uint16_t x, uint16_t  y, uint16_t color);