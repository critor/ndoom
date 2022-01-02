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
//    Nil.
//    
//-----------------------------------------------------------------------------


#ifndef __M_BBOX__
#define __M_BBOX__

#include "m_fixed.h"
/*
NOT NEEDED, ALREADY DEFINED IN DOOMTYPE.H
//Replacement of the values that on other platforms would be in <values.h>
//2147483647 and -2147483647 are probably closer to reality, but let's keep it 16bit for now, eh?
#define MI/NINT 			-32767
#define MAX/INT			 32767

*/
// Bounding box coordinate storage.
enum
{
    BOXTOP,
    BOXBOTTOM,
    BOXLEFT,
    BOXRIGHT
};	// bbox coordinates

// Bounding box functions.
void M_ClearBox (fixed_t*	box);

void
M_AddToBox( fixed_t* box, fixed_t x, fixed_t y );


#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
