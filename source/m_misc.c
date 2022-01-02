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
//
// $Log:$
//
// DESCRIPTION:
//	Main loop menu stuff.
//
//-----------------------------------------------------------------------------

//static const char 

//rcsid[] = "$Id: m_misc.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <os.h>

#include "doomdef.h"

#include "z_zone.h"

#include "m_swap.h"
//#include "m_argv.h"

#include "w_wad.h"

#include "i_system.h"
#include "i_video.h"
#include "v_video.h"

#include "hu_stuff.h"

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"

#include "m_misc.h"

//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern patch_t*		hu_font[HU_FONTSIZE];

int M_DrawText ( int x, int y, boolean direct, char* string )
{
    int 	c;
    int		w;

    while (*string)
    {
		c = toupper_int(*string) - HU_FONTSTART;
		string++;
		if (c < 0 || c> HU_FONTSIZE)
		{
			x += 4;
			continue;
		}
			
		w = SHORT (hu_font[c]->width);
		if (x+w > SCREENWIDTH)
			break;
		if (direct)
			V_DrawPatchDirect(x, y, 0, hu_font[c]);
		else
			V_DrawPatch(x, y, 0, hu_font[c]);
		x+=w;
    }
    return x;
}




//
// M_WriteFile
//
boolean M_WriteFile ( char const* name, void* source, int length )
{
    FILE*	handle;
	char	fpath[99];
    int		count;
	
	//Convert the relative path we got into an absolute path
	strcpy(fpath,current_dir);
	strcat(fpath,name);
	printf("savepath: %s \n",fpath);
	
	//handle = open ( name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
	handle= fopen (fpath, "wb");

    if (handle == NULL)
		return false;

    //count = write (handle, source, length);
	count= fwrite (source, length, 1, handle);
    fclose (handle);
	
    /*if (count < length)
		return false;*/
	if (count != 1)
		return false;
		
    return true;
}


//
// M_ReadFile
//
int M_ReadFile ( char const* name, byte** buffer )
{
    int			count, length;
	char		fpath[99];
	FILE*		handle;
    struct stat	fileinfo;
    byte		*buf;
	
	//Convert the relative path we got into an absolute path
	strcpy(fpath,current_dir);
	strcat(fpath,name);
	printf("loadpath: %s \n",fpath);
	
    handle = fopen (fpath, "rb");
    if (handle == NULL)
		I_Error ("Couldn't read file [1] %s", name);
	
    if (stat (fpath,&fileinfo) == -1)
		I_Error ("Couldn't read file [2] %s", name);
	
    length = fileinfo.st_size;
    buf = Z_Malloc (length, PU_STATIC, NULL);
    //count = read (handle, buf, length);
	//fread ( dest, size elements, count elements, FILE handle );
	count = fread (buf, length, 1, handle);
    fclose (handle);
	
    /*if (count < length)
		I_Error ("Couldn't read file %s", name);
	*/
	
	if (count != 1)
		I_Error ("Couldn't read file [3] %s", name);
	
    *buffer = buf;
	
    return length;
}


//
// DEFAULTS
//
extern int	key_right;
extern int	key_left;
extern int	key_up;
extern int	key_down;

extern int	key_strafeleft;
extern int	key_straferight;

extern int	key_fire;
extern int	key_use;
extern int	key_strafe;
extern int	key_speed;

extern int	viewwidth;
extern int	viewheight;

extern int	showMessages;

extern int	detailLevel;

extern int	screenblocks;

extern int	showMessages;

int		numdefaults;
char*	defaultfile;