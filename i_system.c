// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

//static const char rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";




#include <os.h>

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"

#include "g_game.h"
#include "d_main.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"

#include "c_common.h" // CX port
#include "touchpad.h" // CX port


int 			contrast_change= 0;
int 			map_mode= 0;
int				mb_used = 6;

unsigned char		timer_def_8;	// 8-bits
unsigned short int 	timer_def_16;	// 16-bits
unsigned long int	timer_def_32_1;	// 32-bits
unsigned long int	timer_def_32_2;	// 32-bits


//unsigned short 	timer_def_one;	// 16-bits
//byte			timer_def_two;
byte*			mb_mainzone;

//Real Time Clock register (get current time in seconds - READ ONLY)
//REG_RTIME = (volatile unsigned int*)(MEM_BASE + 0x90000);
#define REG_RTIME (volatile unsigned*)(MEM_BASE + 0x90000)
//Timer 1 register (~488Hz = ~7x70)
//REG_ATIME = (volatile unsigned int*)(MEM_BASE + 0xC000C);
#define REG_ATIME (volatile unsigned*)(MEM_BASE + 0xC000C)
//Timer 2 register (~100Hz)
//REG_BTIME = (volatile unsigned int*)(MEM_BASE + 0xD000C);
//define REG_BTIME (volatile unsigned*)(MEM_BASE + 0xD000C)
#define REG_BTIME (volatile unsigned*)(MEM_BASE + (has_colors?0xD0004:0xD000C)) // CX port

ticcmd_t	emptycmd;
ticcmd_t*	I_BaseTiccmd(void)
{
    return &emptycmd;
}


int  I_GetHeapSize (void)
{
    return mb_used*1024*1024;
}

byte* I_ZoneBase (int*	size)
{
    *size = mb_used*1024*1024;
	printf ("Allocating six megs of memory, hold on... \n");
    mb_mainzone = (byte *) malloc (*size);
	return mb_mainzone;
}

//
// I_Init
// Initialize machine state
//
void I_Init (void)
{	//Store timer values
	if(!has_colors) // CX port
	{	timer_def_16= *(volatile unsigned short int*)0x900D0010;
		timer_def_8= *(volatile unsigned char*)0x900D0014;
		//Initialize timers
		*(volatile unsigned short*)0x900D0010= 1024;//8184;//32768; // Timer divider
		*(volatile byte*)0x900D0014= 0xF;					// 0b1111	Timer control: freeze + increase + infinite
	} // CX port
	else
	{
		timer_def_32_1= *(volatile unsigned long int*)0x900D0000;
		timer_def_32_2= *(volatile unsigned long int*)0x900D0004;
		timer_def_8= *(volatile unsigned char*)0x900D0020;

printf("init B-timer mode: %u\n",timer_def_8);
printf("init B-timer value: %lu\n",timer_def_32_2);
printf("init B-timer top: %lu\n",timer_def_32_1);

		//Initialize timers
		*(volatile unsigned long int*)0x900D0000= 0xFFFFFFFF;
//		*(volatile unsigned char*)0x900D0020= (timer_def_8 & ~0b00000100) | 0b10001000;
	}
}


//
// I_Restore
// Restore machine state (if not done, the calculator will be killed once it enters sleep mode after the game quit)
//
void I_Restore (void)
{
	//Restore timer values
	if(!has_colors) // CX port
	{	*(volatile unsigned short int*)0x900D0010= timer_def_16;
		*(volatile unsigned char*)0x900D0014= timer_def_8;
	} // CX port
	else
	{
//		*(volatile unsigned long int*)0x900D0000= timer_def_32_2;
//		*(volatile unsigned char*)0x900D0020= timer_def_8;
		*(volatile unsigned long int*)0x900D0000= timer_def_32_1;
	}
}


//
// I_GetTime
//
int  I_GetTime (void)
{
	unsigned long int rb = *REG_BTIME;
	if(has_colors)
	{	rb = 0xFFFFFFFF-rb;
		rb = rb/64;
	}
//printf("Time: %lu\n",rb);
	static int			c_time= 0;
    static unsigned int	c_temp= 0xFFFF;
	int  				c_delta= 0;
	
	//Check for overflow
	if (rb<c_temp)
		c_delta= 0;//(*REG_BTIME + (0xFF - c_time)); //fuck, i just can't figure out this overflow
	else
		c_delta= rb - c_temp;
	
	c_temp = rb;
	c_time+= c_delta;
	//printf("t: %i \n",time);
	return c_time;//c_time/600;
}

//
// I_Quit
//
void I_Quit (void)
{	//M_SaveDefaults ();
	free(mb_mainzone);
	endTP(); // CX port
	fuck= true;
}

//
// I_GetMainDir
// Processes argv[0], saves current directory path
//
void I_GetMainDir (char* dir)
{
	short i,j= 0;
	i= strlen(dir);
	
	while (i>0) {
	if (j==0 && dir[i]==47)
		j= i;
	i--;
	};
	
	current_dir= malloc(j+2);
	strncpy(current_dir,dir,j+1);
}

//
// toupper_int
//
int toupper_int(int i) {
	if(('a' <= i) && (i <= 'z'))
		return 'A' + (i - 'a');
	else
		return i;
}

//
// access_ok
//
boolean access_ok(const char *file) {
	struct stat buffer;
	int         status;
	
	//Access the file
	status= stat(file,&buffer);
	
	//Check if the OK value got returned
	if (status==0)
		return true;
	return false;
}

extern unsigned int* orig_address;	// CX port
extern void*	LCD_BUFFER;

//
// I_Error
//
extern boolean demorecording;

void I_Warning (char *error, ...)
{
/*    va_list	argptr;
	char*	ferror=malloc(512); // CX port

    // Message first.
    va_start (argptr,error);
    printf ("Error: ");
    printf (error,argptr);
    printf ("\n");
    sprintf(ferror,error,argptr);
    va_end (argptr);

	//Inform the user something bad had occured
	resetConsole();
	displnBuf(SCREEN_BASE_ADDRESS,ferror,1,0); // CX port
//	I_DisableGraphics(); // CX port
//	show_msgbox("FUCK!",ferror); // CX port
//	I_EnableGraphics(); // CX port
	//I_ShowDBox("FUCK!", error);
	free(ferror); // CX port
*/
}

void I_Error (char *error, ...)
{
/*    va_list	argptr;
	char*	ferror=malloc(512); // CX port

    // Message first.
    va_start (argptr,error);
    printf ("Error: ");
    printf (error,argptr);
    printf ("\n");
    sprintf(ferror,error,argptr);
    va_end (argptr);

	resetConsole();
	//Inform the user something bad had occured
	displnBuf(SCREEN_BASE_ADDRESS,ferror,1,0); // CX port
//	I_DisableGraphics(); // CX port
//	show_msgbox("FUCK!",ferror); // CX port
//	I_EnableGraphics(); // CX port
	displnBuf(SCREEN_BASE_ADDRESS,"Press any key to continue...",1,0);
	while(any_key_pressed());
	while(!any_key_pressed());
	//I_ShowDBox("FUCK!", error);
	free(ferror); // CX port
*/
	
	
    // Shutdown. Here might be other errors.
    if (demorecording)
		G_CheckDemoStatus();
    
	//Exit function here
	I_Quit();
}

//
// I_StartTic
// Processes events
// Is a bit strange because a PC handles events synchronously, while the Nspire asynchronously
//
void I_StartTic (void)
{	int touchedzone=0;
	readTP();
	touchedzone=getTouchedZone9();

	event_t event;	

	if (isKeyPressed(KEY_NSPIRE_CLICK) || isKeyPressed(KEY_NSPIRE_EQU) || isKeyPressed(KEY_NSPIRE_TRIG) || isKeyPressed(KEY_NSPIRE_BAR) || isTPPressed()) {
		event.type= ev_keydown;
		event.data1 = KEY_RSHIFT;
		D_PostEvent(&event);
	} else {
		event.type= ev_keyup;
		event.data1 = KEY_RSHIFT;
		D_PostEvent(&event);
	}

	if (isKeyPressed(KEY_NSPIRE_PLUS) && ((*(volatile unsigned*)LCD_CNTRST)&0b11111111)<255) {
		if (!contrast_change)
		{
			*(volatile unsigned*)LCD_CNTRST= (*(volatile unsigned*)LCD_CNTRST + 1);
//			contrast_change= 1; // CX port
		}
	} else if (isKeyPressed(KEY_NSPIRE_MINUS) && ((*(volatile unsigned*)LCD_CNTRST)&0b11111111)>0) {
		if (!contrast_change)
		{
			*(volatile unsigned*)LCD_CNTRST= (*(volatile unsigned*)LCD_CNTRST - 1);
//			contrast_change= 1; // CX port
		}
	} else {
		contrast_change= 0;
	}

	if (isKeyPressed(KEY_NSPIRE_HOME) || isKeyPressed(KEY_NSPIRE_QUES) || isKeyPressed(KEY_NSPIRE_QUESEXCL) || isKeyPressed(KEY_NSPIRE_FLAG)) {
		if(!map_mode)
		{	event.type= ev_keydown;
			event.data1 = KEY_TAB;
			D_PostEvent(&event);
			map_mode = 1; // CX port
		}
	} else {
		event.type= ev_keyup;
		event.data1 = KEY_TAB;
		D_PostEvent(&event);
		map_mode = 0; // CX port
	}

	if (isKeyPressed(KEY_NSPIRE_VAR))
		G_SaveGame(0,doomsav); // CX port

	if (isKeyPressed(KEY_NSPIRE_CAT) || isKeyPressed(KEY_NSPIRE_FRAC))
		G_LoadGame(doomsav); // CX port

	//For movement+shooting, differentiate between Touchpad/Clickpad/Emulator

		if (isKeyPressed(KEY_NSPIRE_SHIFT) || isKeyPressed(KEY_NSPIRE_SPACE) || isKeyPressed(KEY_NSPIRE_0) || isKeyPressed(KEY_NSPIRE_ENTER) || isKeyPressed(KEY_NSPIRE_RET)) {
			event.type= ev_keydown;
			event.data1 = ' ';
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = ' ';
			D_PostEvent(&event);
		}

		if (isKeyPressed(KEY_NSPIRE_CTRL) || isKeyPressed(KEY_NSPIRE_DEL) || isKeyPressed(KEY_NSPIRE_5) || isKeyPressed(KEY_NSPIRE_PERIOD) || isKeyPressed(KEY_NSPIRE_NEGATIVE)) {
			event.type= ev_keydown;
			event.data1 = KEY_RCTRL;
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = KEY_RCTRL;
			D_PostEvent(&event);
		}

		//HAX HAX HAX
		if (isKeyPressed(KEY_NSPIRE_ESC) || isKeyPressed(KEY_NSPIRE_ON))
			I_Quit();
		//HAX HAX HAX

		if (isKeyPressed(KEY_NSPIRE_UP) || isKeyPressed(KEY_NSPIRE_LEFTUP) || isKeyPressed(KEY_NSPIRE_UPRIGHT) || isKeyPressed(KEY_NSPIRE_7) || isKeyPressed(KEY_NSPIRE_8) || isKeyPressed(KEY_NSPIRE_9) || touchedzone==7 || touchedzone==8 || touchedzone==9) {
			event.type= ev_keydown;
			event.data1 = KEY_UPARROW;
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = KEY_UPARROW;
			D_PostEvent(&event);
		}
		
		if (isKeyPressed(KEY_NSPIRE_DOWN) || isKeyPressed(KEY_NSPIRE_DOWNLEFT) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN) || isKeyPressed(KEY_NSPIRE_1) || isKeyPressed(KEY_NSPIRE_2) || isKeyPressed(KEY_NSPIRE_3) || touchedzone==1 || touchedzone==2 || touchedzone==3) {
			event.type= ev_keydown;
			event.data1 = KEY_DOWNARROW;
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = KEY_DOWNARROW;
			D_PostEvent(&event);
		}
		
		if (isKeyPressed(KEY_NSPIRE_LEFT) || isKeyPressed(KEY_NSPIRE_DOWNLEFT) || isKeyPressed(KEY_NSPIRE_LEFTUP) || isKeyPressed(KEY_NSPIRE_1) || isKeyPressed(KEY_NSPIRE_4) || isKeyPressed(KEY_NSPIRE_7) || touchedzone==1 || touchedzone==4 || touchedzone==7) {
			event.type= ev_keydown;
			event.data1 = KEY_LEFTARROW;
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = KEY_LEFTARROW;
			D_PostEvent(&event);
		}
		
		if (isKeyPressed(KEY_NSPIRE_RIGHT) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN)  || isKeyPressed(KEY_NSPIRE_UPRIGHT)|| isKeyPressed(KEY_NSPIRE_3) || isKeyPressed(KEY_NSPIRE_6) || isKeyPressed(KEY_NSPIRE_9) || touchedzone==3 || touchedzone==6 || touchedzone==9) {
			event.type= ev_keydown;
			event.data1 = KEY_RIGHTARROW;
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = KEY_RIGHTARROW;
			D_PostEvent(&event);
		}

		if (isKeyPressed(KEY_NSPIRE_A)) {
			event.type= ev_keydown;
			event.data1 = '1';
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = '1';
			D_PostEvent(&event);
		}
		if (isKeyPressed(KEY_NSPIRE_B)) {
			event.type= ev_keydown;
			event.data1 = '2';
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = '2';
			D_PostEvent(&event);
		}
		if (isKeyPressed(KEY_NSPIRE_C)) {
			event.type= ev_keydown;
			event.data1 = '3';
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = '3';
			D_PostEvent(&event);
		}
		if (isKeyPressed(KEY_NSPIRE_D)) {
			event.type= ev_keydown;
			event.data1 = '4';
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = '4';
			D_PostEvent(&event);
		}
		if (isKeyPressed(KEY_NSPIRE_E)) {
			event.type= ev_keydown;
			event.data1 = '5';
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = '5';
			D_PostEvent(&event);
		}
		if (isKeyPressed(KEY_NSPIRE_F)) {
			event.type= ev_keydown;
			event.data1 = '6';
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = '6';
			D_PostEvent(&event);
		}
		if (isKeyPressed(KEY_NSPIRE_G)) {
			event.type= ev_keydown;
			event.data1 = '7';
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = '7';
			D_PostEvent(&event);
		}

		if (isKeyPressed(KEY_NSPIRE_DOC) || isKeyPressed(KEY_NSPIRE_TAB) || isKeyPressed(KEY_NSPIRE_EXP) || isKeyPressed(KEY_NSPIRE_MULTIPLY)) {
			event.type= ev_keydown;
			event.data1 = KEY_SLEFTARROW;
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = KEY_SLEFTARROW;
			D_PostEvent(&event);
		}

		if (isKeyPressed(KEY_NSPIRE_SCRATCHPAD) || isKeyPressed(KEY_NSPIRE_MENU) || isKeyPressed(KEY_NSPIRE_DIVIDE) || isKeyPressed(KEY_NSPIRE_SQU)) {
			event.type= ev_keydown;
			event.data1 = KEY_SRIGHTARROW;
			D_PostEvent(&event);
		} else {
			event.type= ev_keyup;
			event.data1 = KEY_SRIGHTARROW;
			D_PostEvent(&event);
		}

}
