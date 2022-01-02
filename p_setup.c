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
//	Do all the WAD I/O, get map description,
//	set up initial state and misc. LUTs.
//
//-----------------------------------------------------------------------------

//static const char 

//rcsid[] = "$Id: p_setup.c,v 1.5 1997/02/03 22:45:12 b1 Exp $";


//#include <math.h>

#include "z_zone.h"

#include "m_swap.h"
#include "m_bbox.h"

#include "g_game.h"

#include "i_system.h"
#include "w_wad.h"

#include "doomdef.h"
#include "p_local.h"

//#include "s_sound.h"

#include "doomstat.h"


void	P_SpawnMapThing (mapthing_t*	mthing);


//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int				numvertexes;
vertex_t*		vertexes;

int				numsegs;
seg_t*			segs;

int				numsectors;
sector_t*		sectors;

int				numsubsectors;
subsector_t*	subsectors;

int				numnodes;
node_t*			nodes;

int				numlines;
line_t*			lines;

int				numsides;
side_t*			sides;


// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int		bmapwidth;
int		bmapheight;	// size in mapblocks
short*		blockmap;	// int for larger maps
// offsets in blockmap are from here
short*		blockmaplump;		
// origin of block map
fixed_t		bmaporgx;
fixed_t		bmaporgy;
// for thing chains
mobj_t**	blocklinks;		


// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
byte*		rejectmatrix;


// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS	10

mapthing_t	deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_t*	deathmatch_p;
mapthing_t	playerstarts[MAXPLAYERS];





//
// P_LoadVertexes
//
void P_LoadVertexes (int lump)
{
    byte*		data;
    int			i;
    mapvertex_t*	ml;
    vertex_t*		li;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);	

    // Load data into cache.
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    ml = (mapvertex_t *)data;
    li = vertexes;

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i=0 ; i<numvertexes ; i++, li++, ml++)
    {
		li->x = SHORT(ml->x)<<FRACBITS;
		li->y = SHORT(ml->y)<<FRACBITS;
    }

    // Free buffer memory.
    Z_Free (data);
}



//
// P_LoadSegs
//
void P_LoadSegs (int lump)
{
    byte*		data;
    int			i;
    mapseg_t*	ml;
    seg_t*		li;
    line_t*		ldef;
    int			linedef;
    int			side;
	int         sidenum;
	
    numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
    segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);	
    memset (segs, 0, numsegs*sizeof(seg_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    ml = (mapseg_t *)data;
    li = segs;
    for (i=0 ; i<numsegs ; i++, li++, ml++)
    {
		li->v1 = &vertexes[SHORT(ml->v1)];
		li->v2 = &vertexes[SHORT(ml->v2)];
		
		li->angle = (SHORT(ml->angle))<<16;
		li->offset = (SHORT(ml->offset))<<16;
		linedef = SHORT(ml->linedef);
		ldef = &lines[linedef];
		li->linedef = ldef;
		side = SHORT(ml->side);
		li->sidedef = &sides[ldef->sidenum[side]];
		li->frontsector = sides[ldef->sidenum[side]].sector;
		if (ldef-> flags & ML_TWOSIDED)
		{
			sidenum = ldef->sidenum[side ^ 1];
			//li->backsector = sides[ldef->sidenum[side^1]].sector;
			
			if (sidenum < 0 || sidenum >= numsides)
            {
                sidenum = 0;
            }
			li->backsector = sides[sidenum].sector;
		}
		else
			li->backsector = 0;
		
		/*
		//LineDef8, E1M2
		if (i==761)
		{
			printf("| ldef->sidenum[side] %i \n",ldef->sidenum[side]);
			printf("| li->v1, %i, %i \n",li->v1->x>>16,li->v1->y>>16);
			printf("| li->v2, %i, %i \n",li->v2->x>>16,li->v2->y>>16);
			printf("| li->sidedef->sector %p \n",li->sidedef->sector);
			printf("| li->frontsector %p \n",li->frontsector);
			printf("| li->backsector %p \n",li->backsector);
			printf("| sector->floorheight %i \n",li->sidedef->sector->floorheight>>16);
			printf("| sector->ceilingheight %i \n",li->sidedef->sector->ceilingheight>>16);
			printf("| sector->lightlevel %i \n",li->sidedef->sector->lightlevel);
			//BAD SECTOR ALERT!!! The Fuxxoration already appears here...
			//Possible Points of Failure:
			//	[X] ldef (lines[] filled with garbage prior) - THE BELOW WORKS, SO THIS MUST TOO
			//	[X] ldef->sidenum[side] (sidenum[] filled with garbage prior) - CORRECT SIDEDEF IS INDEED LOADED
			//	[X] sides[] filled with garbage prior to this - AHA!
		}
		*/
		
		/*if (i==150)
		{
			printf("| li->v1, %i, %i \n",li->v1->x>>16,li->v1->y>>16);
			printf("| li->v2, %i, %i \n",li->v2->x>>16,li->v2->y>>16);
			printf("| id %i; %X \n",i,ldef-> flags);
			printf("| twosided, %i \n",(ldef-> flags & ML_TWOSIDED));
		}
		else
			printf("id %i; %X \n",i,ldef-> flags);*/
    }
	
    Z_Free (data);
}


//
// P_LoadSubsectors
//
void P_LoadSubsectors (int lump)
{
    byte*		data;
    int			i;
    mapsubsector_t*	ms;
    subsector_t*	ss;
	
    numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    ms = (mapsubsector_t *)data;
    memset (subsectors,0, numsubsectors*sizeof(subsector_t));
    ss = subsectors;
    
    for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
    {
		ss->numlines = SHORT(ms->numsegs);
		ss->firstline = SHORT(ms->firstseg);
		//printf("^%i|%i|%i \n",i,ss->numlines,ss->firstline);
    }
	
    Z_Free (data);
}



//
// P_LoadSectors
//
void P_LoadSectors (int lump)
{
    byte*			data;
    int				i;
    mapsector_t*	ms;
    sector_t*		ss;
	char*			tmp= "        ";
	
    numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
    sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);	
    memset (sectors, 0, numsectors*sizeof(sector_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    ms = (mapsector_t *)data;
    ss = sectors;
	
    for (i=0 ; i<numsectors ; i++, ss++, ms++)
    {
		//Some funkiness has to be done because the floorpic and ceilingpic definitions are missing their NULL terminators...
		ss->floorheight = SHORT(ms->floorheight)<<FRACBITS;
		ss->ceilingheight = SHORT(ms->ceilingheight)<<FRACBITS;
		//printf("S%if%i|c%i\n",i,ss->floorheight>>16,ss->ceilingheight>>16);
		strncpy(tmp,ms->floorpic,8);
		//printf ("tmp= %s \n",tmp);
		/*printf ("ms->floorpic= %s \n",ms->floorpic);
		printf ("tmp= %s \n",&tmp);
		printf ("ms->ceilingpic= %s \n",ms->ceilingpic);
		ss->floorpic = R_FlatNumForName(ms->floorpic);
		ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);*/
		ss->floorpic = R_FlatNumForName(tmp);
		strncpy(tmp,ms->ceilingpic,8);
		//printf ("tmp= %s \n",tmp);
		ss->ceilingpic = R_FlatNumForName(tmp);
		//printf("gotf: %i, gotc: %i \n",ss->floorpic,ss->ceilingpic);
		ss->lightlevel = SHORT(ms->lightlevel);
		ss->special = SHORT(ms->special);
		ss->tag = SHORT(ms->tag);
		ss->thinglist = NULL;
    }
	
    Z_Free (data);
}


//
// P_LoadNodes
//
void P_LoadNodes (int lump)
{
    byte*	data;
    int		i;
    int		j;
    int		k;
    mapnode_t*	mn;
    node_t*	no;
	
    numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
    nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    mn = (mapnode_t *)data;
    no = nodes;
    
    for (i=0 ; i<numnodes ; i++, no++, mn++)
    {
	no->x = SHORT(mn->x)<<FRACBITS;
	no->y = SHORT(mn->y)<<FRACBITS;
	no->dx = SHORT(mn->dx)<<FRACBITS;
	no->dy = SHORT(mn->dy)<<FRACBITS;
	for (j=0 ; j<2 ; j++)
	{
	    no->children[j] = SHORT(mn->children[j]);
	    for (k=0 ; k<4 ; k++)
		no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
	}
    }
	
    Z_Free (data);
}


//
// P_LoadThings
//
void P_LoadThings (int lump)
{
    byte*		data;
    int			i;
    mapthing_t*		mt;
    int			numthings;
    boolean		spawn;
	
    data = W_CacheLumpNum (lump,PU_STATIC);
    numthings = W_LumpLength (lump) / sizeof(mapthing_t);
	
    mt = (mapthing_t *)data;
    for (i=0 ; i<numthings ; i++, mt++)
    {
		spawn = true;

		// Do not spawn cool, new monsters if !commercial
		if ( gamemode != commercial)
		{
			switch(mt->type)
			{
			  case 68:	// Arachnotron
			  case 64:	// Archvile
			  case 88:	// Boss Brain
			  case 89:	// Boss Shooter
			  case 69:	// Hell Knight
			  case 67:	// Mancubus
			  case 71:	// Pain Elemental
			  case 65:	// Former Human Commando
			  case 66:	// Revenant
			  case 84:	// Wolf SS
			spawn = false;
			break;
			}
		}
		if (spawn == false)
			break;

		// Do spawn all other stuff. 
		mt->x = SHORT(mt->x);
		mt->y = SHORT(mt->y);
		mt->angle = SHORT(mt->angle);
		mt->type = SHORT(mt->type);
		mt->options = SHORT(mt->options);
		
		/*if (i==207)
		{
			printf("mt->x: %i\n",SHORT(mt->x));
			printf("mt->y: %i\n",SHORT(mt->y));
		}*/
		
		P_SpawnMapThing (mt);
    }
	
    Z_Free (data);
}


//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs (int lump)
{
    byte*		data;
    int			i;
    maplinedef_t*	mld;
    line_t*		ld;
    vertex_t*		v1;
    vertex_t*		v2;
	
    numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
    lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);	
    memset (lines, 0, numlines*sizeof(line_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    mld = (maplinedef_t *)data;
    ld = lines;
    for (i=0 ; i<numlines ; i++, mld++, ld++)
    {
	ld->flags = SHORT(mld->flags);
	ld->special = SHORT(mld->special);
	ld->tag = SHORT(mld->tag);
	v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
	v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
	ld->dx = v2->x - v1->x;
	ld->dy = v2->y - v1->y;
	
	if (!ld->dx)
	    ld->slopetype = ST_VERTICAL;
	else if (!ld->dy)
	    ld->slopetype = ST_HORIZONTAL;
	else
	{
	    if (FixedDiv (ld->dy , ld->dx) > 0)
		ld->slopetype = ST_POSITIVE;
	    else
		ld->slopetype = ST_NEGATIVE;
	}
		
	if (v1->x < v2->x)
	{
	    ld->bbox[BOXLEFT] = v1->x;
	    ld->bbox[BOXRIGHT] = v2->x;
	}
	else
	{
	    ld->bbox[BOXLEFT] = v2->x;
	    ld->bbox[BOXRIGHT] = v1->x;
	}

	if (v1->y < v2->y)
	{
	    ld->bbox[BOXBOTTOM] = v1->y;
	    ld->bbox[BOXTOP] = v2->y;
	}
	else
	{
	    ld->bbox[BOXBOTTOM] = v2->y;
	    ld->bbox[BOXTOP] = v1->y;
	}

	ld->sidenum[0] = SHORT(mld->sidenum[0]);
	ld->sidenum[1] = SHORT(mld->sidenum[1]);

	if (ld->sidenum[0] != -1)
	    ld->frontsector = sides[ld->sidenum[0]].sector;
	else
	    ld->frontsector = 0;

	if (ld->sidenum[1] != -1)
	    ld->backsector = sides[ld->sidenum[1]].sector;
	else
	    ld->backsector = 0;
    }
	
    Z_Free (data);
}


//
// P_LoadSideDefs
//
void P_LoadSideDefs (int lump)
{
    byte*		data;
    int			i;
    mapsidedef_t*	msd;
    side_t*		sd;
	
    numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);	
    memset (sides, 0, numsides*sizeof(side_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    msd = (mapsidedef_t *)data;
    sd = sides;
    for (i=0 ; i<numsides ; i++, msd++, sd++)
    {
		
		/*//The buggy sidedefs
		if (i==8 || i==154)
		{
			printf("sidedef%i: sector=%i, \n",i,msd->sector);
			//printf("_sector%i: normal=%i, SHORT()=%i \n",i,msd->sector,SHORT(msd->sector));
		}*/
		//HAX HAX HAX
		//I've got no idea why it's getting changed below, so I'll just move it here...
		sd->sector = &sectors[SHORT(msd->sector)];
		
		sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
		sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;
		sd->toptexture = R_TextureNumForName(msd->toptexture);
		sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
		sd->midtexture = R_TextureNumForName(msd->midtexture);
		//sd->sector = &sectors[SHORT(msd->sector)];
		//printf("$SIDEDEF %i IS IN SECTOR %i\n",i,SHORT(msd->sector)); //error already here!
		
		
		/*//OK, WTF? This should be the FUCKING SAME AS THE THING ABOVE!1!!
		if (i==8 || i==154)
		{
			printf("sidedef%i: sector=%i, \n",i,msd->sector);
			//printf("_sector%i: normal=%i, SHORT()=%i \n",i,msd->sector,SHORT(msd->sector));
			///AAAAAAAAAAAAAAAARGH WTF!!!! HOW CAN IT BE DIFFERENT?! HOW?! GOD, PLEASE TELL ME, HOW??!!
		}*/
		
		/*
		sd->sector CORRECTLY ASSIGNED HERE into the sides[] array!
		AHA! BUT ONLY 99% of the time!!!
		SIDEDEF8->sector should be 103 (0110 0111),
		but instead is set to       73 (0100 0111)!
		...
		WTF?! ONE BIT?! ALL THESE HEADACHES TO DUE ONE SINGLE STINKING BIT????!!!!
		AAAAAAAAAAAAAAAAAAAAARGH!
		I'm guessing it's getting AND'ed with 0x20 for some mysterious reason...
		*/
    }
	
    Z_Free (data);
}


//
// P_LoadBlockMap
//
void P_LoadBlockMap (int lump)
{
    int i;
    int count;
    int lumplen;

    lumplen = W_LumpLength(lump);
    count = lumplen / 2;
	
    blockmaplump = Z_Malloc(lumplen, PU_LEVEL, NULL);
    W_ReadLump(lump, blockmaplump);
    blockmap = blockmaplump + 4;

    // Swap all short integers to native byte ordering.
    for (i=0; i<count; i++)
		blockmaplump[i] = SHORT(blockmaplump[i]);
		
    // Read the header

    bmaporgx = blockmaplump[0]<<FRACBITS;
    bmaporgy = blockmaplump[1]<<FRACBITS;
    bmapwidth = blockmaplump[2];
    bmapheight = blockmaplump[3];
	
    // Clear out mobj chains

    count = sizeof(*blocklinks) * bmapwidth * bmapheight;
    blocklinks = Z_Malloc(count, PU_LEVEL, 0);
    memset(blocklinks, 0, count);
}
/*
void P_LoadBlockMap (int lump)
{
    int		i;
    int		count;
	
    blockmaplump = W_CacheLumpNum (lump,PU_LEVEL);
    blockmap = blockmaplump+4;
    count = W_LumpLength (lump)/2;

    for (i=0 ; i<count ; i++)
		blockmaplump[i] = SHORT(blockmaplump[i]);
		
    bmaporgx = blockmaplump[0]<<FRACBITS;
    bmaporgy = blockmaplump[1]<<FRACBITS;
    bmapwidth = blockmaplump[2];
    bmapheight = blockmaplump[3];
	
    // clear out mobj chains
    count = sizeof(*blocklinks)* bmapwidth*bmapheight;
    blocklinks = Z_Malloc (count,PU_LEVEL, 0);
    memset (blocklinks, 0, count);
}
*/


//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines (void)
{
    line_t**		linebuffer;
    int				i;
    int				j;
    int				total;
    line_t*			li;
    sector_t*		sector;
    subsector_t*	ss;
    seg_t*			seg;
    fixed_t			bbox[4];
    int				block;
	
    // look up sector number for each subsector
    ss = subsectors;
    for (i=0 ; i<numsubsectors ; i++, ss++)
    {
		seg = &segs[ss->firstline];
		ss->sector = seg->sidedef->sector;
		/*if (ss->firstline==761)
			printf("|I%iZ%iZ%iZ%i\n",i,ss->sector->floorheight>>16,ss->sector->ceilingheight>>16,ss->sector->lightlevel);*/
		/* DEBUG NOTE:
		-The floorheight/ceilingheight variables get fuxxored BEFORE this step
		-Well, either that, or the &seg[ss->firstline] list is failing...
		*/
    }

    // count number of lines in each sector
    li = lines;
    total = 0;
    for (i=0 ; i<numlines ; i++, li++)
    {
	total++;
	li->frontsector->linecount++;

	if (li->backsector && li->backsector != li->frontsector)
	{
	    li->backsector->linecount++;
	    total++;
	}
    }
	
    // build line tables for each sector	
    linebuffer = Z_Malloc (total*4, PU_LEVEL, 0);
    sector = sectors;
    for (i=0 ; i<numsectors ; i++, sector++)
    {
	M_ClearBox (bbox);
	sector->lines = linebuffer;
	li = lines;
	for (j=0 ; j<numlines ; j++, li++)
	{
	    if (li->frontsector == sector || li->backsector == sector)
	    {
		*linebuffer++ = li;
		M_AddToBox (bbox, li->v1->x, li->v1->y);
		M_AddToBox (bbox, li->v2->x, li->v2->y);
	    }
	}
	if (linebuffer - sector->lines != sector->linecount)
	    I_Error ("P_GroupLines: miscounted");
			
	// set the degenmobj_t to the middle of the bounding box
	sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
	sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;
		
	// adjust bounding box to map blocks
	block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
	block = block >= bmapheight ? bmapheight-1 : block;
	sector->blockbox[BOXTOP]=block;

	block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
	block = block < 0 ? 0 : block;
	sector->blockbox[BOXBOTTOM]=block;

	block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
	block = block >= bmapwidth ? bmapwidth-1 : block;
	sector->blockbox[BOXRIGHT]=block;

	block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
	block = block < 0 ? 0 : block;
	sector->blockbox[BOXLEFT]=block;
    }
	
}

//
// P_SetupLevel
//
void P_SetupLevel ( int episode, int map, int playermask, skill_t skill)
{
    //int		i;
    char	lumpname[9];
    int		lumpnum;
	
    totalkills = playermask = totalitems = totalsecret = wminfo.maxfrags = 0;
    wminfo.partime = 180;
    //for (i=0 ; i<MAXPLAYERS ; i++)
	players[0].killcount = players[0].secretcount = players[0].itemcount = 0;

    // Initial height of PointOfView
    // will be set by player think.
    players[consoleplayer].viewz = 1; 

    // Make sure all sounds are stopped before Z_FreeTags.
    //S_Start ();

	Z_FreeTags (PU_LEVEL, PU_PURGELEVEL-1);

	
    P_InitThinkers ();
	
	/*
    // if working with a devlopment map, reload it
	WE DO NOT WORK WITH DEVELOPMENT MAPS. WE ARE THE NSPIRED. WE ARE AWESOME. AND LAZY.
    W_Reload ();
	*/
	  
    // find map name
    if ( gamemode == commercial)
    {
		if (map<10)
			sprintf (lumpname,"map0%i", map);
		else
			sprintf (lumpname,"map%i", map);
    }
    else
    {
		lumpname[0] = 'E';
		lumpname[1] = '0' + episode;
		lumpname[2] = 'M';
		lumpname[3] = '0' + map;
		lumpname[4] = 0;
    }
	//I_Error("about to load level!");
    lumpnum = W_GetNumForName (lumpname);
	
    leveltime = 0;
	
    // note: most of this ordering is important	
	printf ("lumpname= %s \n",lumpname);
	
    P_LoadBlockMap (lumpnum+ML_BLOCKMAP);
    P_LoadVertexes (lumpnum+ML_VERTEXES);
    P_LoadSectors (lumpnum+ML_SECTORS); //This loads fine
    P_LoadSideDefs (lumpnum+ML_SIDEDEFS); //sd->sector seems to be assigned correctly here

    P_LoadLineDefs (lumpnum+ML_LINEDEFS);
    P_LoadSubsectors (lumpnum+ML_SSECTORS); //
    P_LoadNodes (lumpnum+ML_NODES);
    P_LoadSegs (lumpnum+ML_SEGS); //FUXXORZ already!!!!! segs[] gets filled with wrong sector data... SOMETIMES!
	
    rejectmatrix = W_CacheLumpNum (lumpnum+ML_REJECT,PU_LEVEL);
    P_GroupLines (); //Looks up segs[] array correctly, but the data INSIDE the array is fucked, therefore it all fucks up

    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;
    P_LoadThings (lumpnum+ML_THINGS);

    // clear special respawning que
    iquehead = iquetail = 0;		
	
    // set up world state
    P_SpawnSpecials ();
	
    // build subsector connect matrix
    //	UNUSED P_ConnectSubsectors ();

    // preload graphics
    if (precache)
		R_PrecacheLevel ();

    printf ("Free memory: %i bytes\n", Z_FreeMemory());
	skill= 0; //Shut up, GCC

}



//
// P_Init
//
void P_Init (void)
{
    P_InitSwitchList ();
    P_InitPicAnims ();
    R_InitSprites ();
}
