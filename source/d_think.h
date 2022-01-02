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
//  MapObj data. Map Objects or mobjs are actors, entities,
//  thinker, take-your-pick... anything that moves, acts, or
//  suffers state changes of more or less violent nature.
//
//-----------------------------------------------------------------------------


#ifndef __D_THINK__
#define __D_THINK__


#ifdef __GNUG__
#pragma interface
#endif



//
// Experimental stuff.
// To compile this as "ANSI C with classes"
//  we will need to handle the various
//  action functions cleanly.
//
//typedef  void (*actionf_v)( );
//typedef  void (*actionf_p1)( void* );
/*typedef  void (*actionf_p2)( void*, void* );

//    think_t		function; commented out from the thinker_t typedef...
typedef union
{
  actionf_p1	acp1;
  actionf_v		acv;
  actionf_p2	acp2;

} actionf_t;*/

//Whoever came up with this shit needs to be shot
//typedef void (*actionf_t)( void* );
//typedef void (*actionf_t)( void*, void* );

/*              ___Explanation of the above___
*
*
*			typedef void (*actionf_t) (arg1, arg2);
*
*	Creates the actionf_t typedef as a synonym for the unwieldy
*	"Pointer to a function returning void and taking two arguments, arg1 and arg2"
*
*	In order to call the function, do:
*			actionf_t (arg1,arg2);
*
*/



//Same for this shit
/*// Historically, "think_t" is yet another
//  function pointer to a routine to handle
//  an actor.
typedef actionf_t  think_t;*/


// Doubly linked list of actors.
typedef struct thinker_s
{
    struct thinker_s*	prev;
    struct thinker_s*	next;
//	actionf_t			function;
	int					function;
	void*				target;
    
} thinker_t;



#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
