#include <os.h>
#include "screen.h"
#include "charmap.h"
#include "tools.h"
#define SCREEN_CONTRAST_ADDR 0x900F0020

#define SCREEN_BASE_PTR		0xC0000010 
#define SCREEN_MODE_ADDR	0xC000001C
#define SCREEN_INT_ADDR		0xC0000020

uint16_t* SCREEN_BASE_ADDR = 0;
uint16_t* orig_screen=0;
uint16_t* malloc_screen=0;
int contrast=0;
uint8_t screen_type = SCR_320x240_565;

uint16_t* getScreen()
{	return SCREEN_BASE_ADDR;
}

void setScreen(uint16_t* buf)
{	
	SCREEN_BASE_ADDR=buf;
	*(volatile unsigned*) SCREEN_BASE_PTR = buf;
}

uint8_t screenType() {
	return screen_type;
}

void initScreen()
{
	if(has_colors && nl_ndless_rev() >= 2004 ) screen_type = lcd_type(); // CX2 port
	setCurColorRGB(0,0,0);
	SCREEN_BASE_ADDR	=*(uint16_t**)SCREEN_BASE_PTR;
	orig_screen = SCREEN_BASE_ADDR;
	contrast=*(volatile unsigned*) SCREEN_CONTRAST_ADDR;
}


uint8_t off_mode=0;
unsigned int setContrast(unsigned int level)
{ 	unsigned int lowlevel = level &0b11111111;
	if(lowlevel>CONTRAST_MAX) lowlevel=CONTRAST_MAX;
	if(lowlevel<CONTRAST_MIN) lowlevel=CONTRAST_MIN;
	level = (level & ~0b11111111)|lowlevel;
	contrast=level;
	if(!has_colors || !off_mode)
		*(volatile unsigned*) SCREEN_CONTRAST_ADDR=level;
	return level;
}

void switchScrOffOn(uint8_t s)
{
	if(!has_colors)
	{
		int mask = 0b100000000001;
		int mode = *(volatile unsigned*) SCREEN_MODE_ADDR;
		if(s)	mode |= mask;
		else	mode &= ~mask;
		*(volatile unsigned*) SCREEN_MODE_ADDR = mode;
	//	return mode&mask;
	}
	else
	{	if(s) off_mode=0;
		else off_mode=1;
		if(!off_mode)
			setContrast(contrast);
		else
			*(volatile unsigned*) SCREEN_CONTRAST_ADDR=(contrast&~0b11111111)|CONTRAST_MIN;
	}
}

void startScreen()
{
	if(!has_colors) {
		int mode = *(volatile unsigned*) SCREEN_MODE_ADDR;
		mode = mode&~0b1110;
		mode = mode|0b1000;
		switchScrOffOn(0); // prevents displaying garbage when the screen mode will be changed
		if(!malloc_screen) {
			malloc_screen=(uint16_t*) malloc(SCREEN_SIZE);
			clrBuf(malloc_screen,0);
			setScreen(malloc_screen);
		}
		*(volatile unsigned*) SCREEN_MODE_ADDR = mode;
		switchScrOffOn(1);
	}
}



void stopScreen()
{	uint16_t* screen=SCREEN_BASE_ADDR;
	if(!has_colors)
	{	int mode = *(volatile unsigned*) SCREEN_MODE_ADDR;
		mode = mode&~0b100001110;
		mode = mode|0b0100;
		switchScrOffOn(0); // prevents displaying garbage when the screen mode will be changed
		*(volatile unsigned*) SCREEN_MODE_ADDR = mode;
		setScreen(orig_screen);
		if(malloc_screen) {
			free(malloc_screen);
			malloc_screen=0;
		}
		clrBuf(SCREEN_BASE_ADDR,0);
		switchScrOffOn(1);
	}
	else {
		if(screen!=orig_screen) {
			memcpy(orig_screen,screen,SCREEN_SIZE);
			setScreen(orig_screen);
		}
	}
} 

void setBufPixel(uint16_t* buf, uint16_t x, uint16_t  y, uint16_t color)
{	if(x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
	{	
		if(screen_type==SCR_320x240_565) {
			buf[y*SCREEN_WIDTH+x]=color;
		}
		else {
			buf[x*SCREEN_HEIGHT+y]=color;			
		}
	}
}

void setBufPixelRGB(uint16_t* buf, uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{	if(x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
	{	
		uint16_t color;
		if(has_colors) color=rgb565(r,g,b);
		else	  color=rgb2gs(r,g,b);
		if(screen_type==SCR_320x240_565) {
			buf[y*SCREEN_WIDTH+x]=color;
		}
		else {
			buf[x*SCREEN_HEIGHT+y]=color;			
		}
	}
}

void convertRGB565toGS(uint16_t* sscreen, uint16_t  w, uint16_t  h) {
	uint16_t  i,j;
	uint16_t* ptr=sscreen;
	uint16_t val;
	uint8_t r,g,b;
	for(j=0;j<h;j++)
		for(i=0;i<w;i++) {
			val=*ptr;
			b=(val&0b11111)<<3;
			g=(val&0b11111100000)>>3;
			r=(val&0b1111100000000000)>>8;
			*ptr=rgb2gs(r,g,b);
			ptr++;
		}
}

void convertRGB565320to240(uint16_t* buffer) {
	uint16_t  i,j;
	uint16_t* ptr=buffer;
	uint16_t* tmpbuffer=malloc(SCREEN_SIZE);
	uint16_t val;
	for(j=0;j<SCREEN_HEIGHT;j++)
		for(i=0;i<SCREEN_WIDTH;i++) {
			val=*ptr;
			tmpbuffer[i*SCREEN_HEIGHT+j]=val;
			ptr++;
		}
	memcpy(buffer,tmpbuffer,SCREEN_SIZE);
	free(tmpbuffer);
}

void convertRGB565(uint16_t* buffer, uint8_t buf_type)
{
	if(!has_colors)
		convertRGB565toGS(buffer,SCREEN_WIDTH,SCREEN_HEIGHT);
	else if(buf_type!=screen_type)
		convertRGB565320to240(buffer);		
}


uint16_t curcolor=0;

void setCurColorRGB(uint8_t r,uint8_t g,uint8_t b) {
	if(!has_colors)
		curcolor=rgb2gs(r,g,b);
	else
		curcolor=rgb565(r,g,b);
}

void putBufChar(uint16_t* buf, uint16_t x, uint16_t y, char ch, uint8_t trsp)
{
  uint8_t i, j;
  uint8_t pixelOn;
  for(i = 0; i < CHAR_HEIGHT; i++)  
  {
    for(j = 0; j < CHAR_WIDTH; j++) 
    {
      pixelOn = charMap_ascii[(unsigned char)ch][i] << j ;  
      pixelOn = pixelOn & 0x80 ;  
	
      if (pixelOn) {
			setBufPixel(buf,x + j, y + i, curcolor); 
      } else if(!trsp) {
			setBufPixel(buf,x + j, y + i, ~curcolor);
      } 
    }
  } 
}

void drwBufStr(uint16_t* buf, uint16_t x, uint16_t y, char* str, uint8_t ret, uint8_t trsp)
{ uint32_t l = strlen(str);
  uint32_t i;
  uint8_t stop=0;
  for (i = 0; i < l && !stop; i++) {
    if (str[i] == 0x0A) {
      if(ret)
      { x = 0;
        y += CHAR_HEIGHT;
      }
      else
      { putBufChar(buf, x,y, ' ',trsp); 
        x += CHAR_WIDTH;
      }
    } else {
      putBufChar(buf, x, y, str[i],trsp);
      x += CHAR_WIDTH;
    }
    if (x >= SCREEN_WIDTH-CHAR_WIDTH)
    { if(ret)
      { x = 0;
        y += CHAR_HEIGHT;
      }
      else
        stop=1;
    }
  }
}

void dispBufImgRGB(uint16_t* buf, int16_t xoff, int16_t yoff, uint8_t* img, uint16_t width, uint16_t height,uint8_t border)
{	uint16_t dwidth=width, dheight=height;
	uint16_t data_x=0, data_y=0;
	uint16_t x = 0, y = 0;
	float i, j;
	if(xoff < 0){
		dwidth = dwidth + xoff;
		data_x = (int)(-xoff);
		xoff = 0;
	}
	if(yoff < 0){
		dheight = dheight + yoff;
		data_y = (int)(-yoff);	
		yoff = 0;
	}
	uint8_t r,g,b;
	for(i=0, x=0; (int)i < dwidth && x < SCREEN_WIDTH; i+= 1, x++)
		for(j=0, y=0; (int)j < dheight && y < SCREEN_HEIGHT; j+= 1, y++)
		{	b=img[(((int)j+data_y)*width+(int)i+data_x)*3];
			g=img[(((int)j+data_y)*width+(int)i+data_x)*3+1];
			r=img[(((int)j+data_y)*width+(int)i+data_x)*3+2];
			if(!border || b!=0 || g!=0 || r!=0)
				setBufPixelRGB(buf, xoff + x, yoff + dheight-1-y, r, g, b);
		}
}

void clrBuf(uint16_t* buf)
{	if(has_colors || malloc_screen)
		clrBufBox(buf,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
	else
		memset(SCREEN_BASE_ADDR,0,SCREEN_PIXELS/2);
}

void clrBufBox(uint16_t* buf, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	uint16_t color=~curcolor;
	uint16_t i,j;
	for(j=0;j<h;j++)
		for(i=0;i<w;i++)
			setBufPixel(buf,x+i,y+j,color);
}

void drwBufHoriz(uint16_t* buf, uint16_t y, uint16_t x1, uint16_t x2)
{	uint16_t m = max(x1,x2);
	uint16_t i = min(x1,x2);
	while(i<=m)
	{	setBufPixel(buf,i,y,curcolor);
		i++;
	}
}

void drwBufVert(uint16_t* buf, uint16_t x, uint16_t y1, uint16_t y2)
{	uint16_t m = max(y1,y2);
	uint16_t i = min(y1,y2);
	while(i<=m)
	{	setBufPixel(buf, x,i,curcolor);
		i++;
	}
}

void drwBufBox(uint16_t* buf, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	drwBufHoriz(buf,y1,x1,x2);
	drwBufHoriz(buf,y2,x1,x2);
	drwBufVert(buf,x1,y1,y2);
	drwBufVert(buf,x2,y1,y2);
}

void drawBufFullBox(uint16_t* buf, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	uint16_t m = max(y1,y2);
	uint16_t i = min(y1,y2);
	while(i<=m)
	{	drwBufHoriz(buf,i,x1,x2);
		i++;
	}
}
