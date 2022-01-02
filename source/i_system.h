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
// DESCRIPTION:
//	System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include "d_ticcmd.h"
#include "d_event.h"

#ifdef __GNUG__
#pragma interface
#endif

// A check to determine which control system to compile for
//#define EMULATOR_CONTROLS 1

#include <os.h>
// Since it isn't defined by ndless for some reason, define the ON key here
//#define KEY_NSPIRE_ON         KEY_(0x10, 0x200)
static const t_key KEY_NSPIRE_ON      = KEYTPAD_(0x10, 0x200, _KEY_DUMMY_ROW, _KEY_DUMMY_COL);

// Called by startup code
// to get the ammount of memory to malloc
// for the zone management.
byte*	I_ZoneBase (int *size);


// Called by D_DoomLoop,
// returns current time in tics.
int I_GetTime (void);

// Initializes the timer used by the function above
void I_Init (void);

// Resets the timer to avoid calckill
void I_Restore (void);

//
// Called by D_DoomLoop,
// called before processing any tics in a frame
// (just after displaying a frame).
// Time consuming syncronous operations
// are performed here (joystick reading).
// Can call D_PostEvent.
//
//void I_StartFrame (void);

//
// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_StartTic (void);

// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
ticcmd_t* I_BaseTiccmd (void);


// Called by M_Responder when quit is selected.
// Clean exit, displays sell blurb.
void I_Quit (void);

// Allocates from low memory under dos,
// just mallocs under unix
byte* I_AllocLow (int length);

//Helper functions
void 	I_ShowDBox(const char* title, const char* msg);
int  	toupper_int(int i);
boolean	access_ok(const char *file);
void	I_GetMainDir(char* dir);
void 	I_Error (char *error, ...);

//Set by I_Error()
extern boolean fuck;

//Set by I_GetMainDir()
extern char* current_dir;

//Access to Nspire registers
#define MEM_BASE		0x90000000
#define LCD_CNTRST		0x900F0020
#define LCD_BASE    	0xC0000000
//define GCNF_ADDR 		0xC000001C
#define GCNF_ADDR 		(has_colors?0xC0000018:0xC000001C) // CX PORT
#define PAL_ADDR  		0xC0000200
#define BFS_ADDR		0xC0000010
#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
