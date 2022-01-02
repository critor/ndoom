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
//	Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------

//static const char 

//rcsid[] = "$Id: i_main.c,v 1.4 1997/02/03 22:45:10 b1 Exp $";



#include "doomdef.h"

//#include "m_argv.h"
#include "d_main.h"

int main (int argc, char* argv[]) 
{  
	//Required because stdout needs the interrupts currently disabled by Ndless
	//Commented because I'm unsure as to how to flush the keyboard buffer after program execution
	//unsigned intmask= TCT_Local_Control_Interrupts(0); // CX port

	initScreen();
	
    D_DoomMain (argc,argv); 
	
	stopScreen();
	argc= 0; //Shut up, GCC
	//TCT_Local_Control_Interrupts(intmask); // CX port
    return 0;
}
