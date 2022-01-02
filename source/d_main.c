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
//	DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//	plus functions to determine game mode (shareware, registered),
//	parse command line parameters, configure game parameters (turbo),
//	and call the startup functions.
//
//-----------------------------------------------------------------------------


//static const char rcsid[] = "$Id: d_main.c,v 1.8 1997/02/03 22:45:09 b1 Exp $";

#define	BGCOLOR		7
#define	FGCOLOR		8


#include <os.h>

#include "doomdef.h"
#include "doomstat.h"

#include "dstrings.h"


#include "z_zone.h"
#include "w_wad.h"
#include "v_video.h"

#include "f_finale.h"
#include "f_wipe.h"

#include "m_misc.h"
#include "m_menu.h"

#include "i_system.h"
#include "i_video.h"

#include "g_game.h"

#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"

#include "p_setup.h"
#include "r_local.h"


#include "d_main.h"

#include "c_common.h" // CX port

#include "charmap.h" // CX port
#include "browse.h" // CX port
#include "screen.h"

//
// D-DoomLoop()
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//
void D_DoomLoop (void);


char*			wadfiles[MAXWADFILES];

char*			current_dir;

boolean			devparm;	// started game with -devparm
boolean         nomonsters;	// checkparm of -nomonsters
boolean         respawnparm;	// checkparm of -respawn
boolean         fastparm;	// checkparm of -fast

boolean         drone;

boolean			singletics = true;

boolean			fuck = false;


extern boolean	inhelpscreens;

skill_t			startskill;
int				startepisode;
int				startmap;
boolean			autostart;

FILE*			debugfile;

boolean			advancedemo;




char		wadfile[1024];		// primary wad file
char		mapdir[1024];       // directory of development maps
char		basedefault[1024];  // default file

void D_ProcessEvents (void);
void G_BuildTiccmd (ticcmd_t* cmd);
void D_DoAdvanceDemo (void);

//
// D_PostEvent
// Called by the I/O functions when input is detected
//
void D_PostEvent (event_t* ev)
{
	if (M_Responder (ev))
		return;               // menu ate the event
	G_Responder (ev);
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t     wipegamestate = GS_DEMOSCREEN;
extern boolean	setsizeneeded;
extern int		showMessages;
void 			R_ExecuteSetViewSize (void);

void D_Display (void)
{
    static  boolean			viewactivestate = false;
    static  boolean			menuactivestate = false;
    static  boolean			inhelpscreensstate = false;
    static  boolean			fullscreen = false;
    static  gamestate_t		oldgamestate = -1;
    static  int				borderdrawcount;
    int						nowtime;
    int						tics;
    int						wipestart;
    int						y;
    boolean					done;
    boolean					wipe;
    boolean					redrawsbar;
		
    redrawsbar = false;
	
	// change the view size if needed
    if (setsizeneeded)
    {
		R_ExecuteSetViewSize ();
		oldgamestate = -1;                      // force background redraw
		borderdrawcount = 3;
    }

    // save the current screen if about to wipe
    if (gamestate != wipegamestate)
    {
		wipe = true;
		wipe_StartScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
    }
    else
		wipe = false;
	
    if (gamestate == GS_LEVEL && gametic)
		HU_Erase();
	
    // do buffered drawing
    switch (gamestate)
    {
      case GS_LEVEL:
		if (!gametic)
			break;
		if (automapactive)
			AM_Drawer ();
		if (wipe || (viewheight != 200 && fullscreen) )
			redrawsbar = true;
		if (inhelpscreensstate && !inhelpscreens)
			redrawsbar = true;              // just put away the help screen
		ST_Drawer (viewheight == 200, redrawsbar );
		fullscreen = viewheight == 200;
		break;

      case GS_INTERMISSION:
		WI_Drawer ();
		break;

      case GS_FINALE:
		F_Drawer ();
		break;

      case GS_DEMOSCREEN:
		D_PageDrawer ();
		break;
    }
	
    // draw the view directly
	if (gamestate == GS_LEVEL && !automapactive && gametic)
		R_RenderPlayerView (&players[0]);
	
    if (gamestate == GS_LEVEL && gametic)
		HU_Drawer ();

    menuactivestate = menuactive;
    viewactivestate = viewactive;
    inhelpscreensstate = inhelpscreens;
    oldgamestate = gamestate;
    
	// draw pause pic
    if (paused)
    {
		if (automapactive)
			y = 4;
		else
			y = viewwindowy+4;
		V_DrawPatchDirect(viewwindowx+(scaledviewwidth-68)/2, y ,0 ,W_CacheLumpName ("M_PAUSE", PU_CACHE) );
    }

    // menus go directly to the screen
    M_Drawer ();          // menu is drawn even on top of everything
	
    // normal update
    if (!wipe)
    {
		I_Flip ();              // page flip or blit buffer
		return;
    }
    
    // wipe update
    wipe_EndScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);

    wipestart = I_GetTime () - 1;

    do
    {
		do
		{
			nowtime = I_GetTime ();
			tics = nowtime - wipestart;
		} while (tics<=0);
		wipestart = nowtime;
		done = wipe_ScreenWipe(wipe_Melt, SCREENWIDTH, SCREENHEIGHT, tics);
		M_Drawer ();							// menu is drawn even on top of wipes
		I_Flip ();								// page flip or blit buffer
    } while (!done);
	gamestate = oldgamestate;
	wipegamestate= oldgamestate;
}



//
//  D_DoomLoop
//
extern  boolean		demorecording;

void D_DoomLoop (void)
{
    if (demorecording)
		G_BeginRecording ();
	
    I_InitGraphics ();

    while (!fuck)
    {
		I_StartTic ();
		
		if (advancedemo)
			D_DoAdvanceDemo ();
		
		M_Ticker (); //Menu ticker
		
		G_Ticker (); //Game ticker
		
		gametic++;

		// Update display, next frame, with current state
		D_Display ();

    }
	I_ShutdownGraphics();
	free(current_dir);
	fclose(lumpinfo->handle);
//printf("%i",-(int)(lumpinfo->handle));
	free(lumpinfo);
	free(lumpcache);
	
	return;
}



//
//  DEMO LOOP
//
int             demosequence;
int             pagetic;
char           *pagename;


//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker (void)
{
    if (--pagetic < 0)
		D_AdvanceDemo ();
}



//
// D_PageDrawer
//
void D_PageDrawer (void)
{
    V_DrawPatch (0,0, 0, W_CacheLumpName(pagename, PU_CACHE));
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
    advancedemo = true;
}

int episode = 1;
int map = 1;
int skill = 2;

//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//
 void D_DoAdvanceDemo (void)
{
	///HAX HAX HAX
	pagename = "TITLEPIC";
	V_DrawPatch (0,0, 0, W_CacheLumpName(pagename, PU_CACHE));
	gamestate= GS_LEVEL;
//	G_DeferedInitNew(2,1,1); // CX port
	G_DeferedInitNew(skill,episode,map); // CX port
	advancedemo= false;
	///HAX HAX HAX
	
    /* UNCOMMENT AFTER DEMO PLAYING WORKS!!!
	
	players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;               // no save / end game here
    paused = false;
    gameaction = ga_nothing;

    if ( gamemode == retail )
      demosequence = (demosequence+1)%7;
    else
      demosequence = (demosequence+1)%6;
	
	
    switch (demosequence)
    {
		case 0:
			if ( gamemode == commercial )
				pagetic = 35 * 11;
			else
				pagetic = 170;
			gamestate = GS_DEMOSCREEN;
			pagename = "TITLEPIC";
			break;
		
		case 1:
			G_DeferedPlayDemo ("demo1");
			break;
		
		case 2:
			pagetic = 200;
			gamestate = GS_DEMOSCREEN;
			pagename = "CREDIT";
			break;
		
		case 3:
			G_DeferedPlayDemo ("demo2");
			break;
		
		case 4:
			gamestate = GS_DEMOSCREEN;
			if ( gamemode == commercial)
			{
				pagetic = 35 * 11;
				pagename = "TITLEPIC";
			}
			else
			{
				pagetic = 200;

				if ( gamemode == retail )
					pagename = "CREDIT";
				else
					pagename = "HELP2";
			}
			break;
		
		case 5:
			G_DeferedPlayDemo ("demo3");
			break;
			// THE DEFINITIVE DOOM Special Edition demo
		
		case 6:
			G_DeferedPlayDemo ("demo4");
			break;
    }*/
}



//
// D_StartTitle
//
void D_StartTitle (void)
{
    gameaction = ga_nothing;
    demosequence = -1;
    D_AdvanceDemo ();
}




//      print title for every printed line
char            title[128];


int nwad=0;

//
// D_AddFile
//
void D_AddFile (char *file)
{	wadfiles[nwad] = file;
	nwad++;
	wadfiles[nwad]=0;
}

void menu(int neps)
{	I_DispHeader();
	drwBufStr(getScreen(), 0, 11*CHAR_HEIGHT, "          [tab] to patch game", 0, 1);
	drwBufStr(getScreen(), 0, 13*CHAR_HEIGHT, "Skill  : [-]                      [+]", 0, 1);
	drwBufStr(getScreen(), 0, 14*CHAR_HEIGHT, "Map    : [C]    [D]", 0, 1);
	drwBufStr(getScreen(), 0, 18*CHAR_HEIGHT, "    For guns and glory press Enter!", 0, 1);
	drwBufStr(getScreen(), 0, 19*CHAR_HEIGHT, "    For work and worry press Esc...", 0, 1);
	if(neps>1)
		drwBufStr(getScreen(), 0,  15*CHAR_HEIGHT, "Episode: [(]   [)]", 0, 1);
}

#define NDLESS_CONFIG	"/documents/ndless/ndless.cfg.tns"

void addshellext(char* extension, char* name)
{	FILE * ndlesscfg = fopen(NDLESS_CONFIG, "r");
	if(ndlesscfg != NULL)
	{	/* Search for extension in ndless.cfg */
		fseek(ndlesscfg, 0, SEEK_END);
		unsigned length = ftell(ndlesscfg);
		rewind(ndlesscfg);
		char * whole_file = (char *) malloc(sizeof(char) * (length + 1));
		memset(whole_file, 0, length);
		fread(whole_file, sizeof(char), length, ndlesscfg);
		whole_file[length] = 0;
		char * line = whole_file;
		char * eol = whole_file;
		int found = 0;
		while((unsigned)(eol - whole_file) < length)
		{	eol = strchr(line, '\n');
			*eol = 0;
			char * point = strchr(line, '.');
			if(point)
			{	*point = 0;
				char * ext = point + 1;
				char * equal = strchr(ext, '=');
				if(equal)
				{	*equal = 0;
					if(strcmp(ext, extension) == 0)
					{	found = 1;	      
						break;
					}
				}
			}
			line = eol + 1;
		}
		/* If not found, add the extension */
		if(!found)
		{	fclose(ndlesscfg);
			ndlesscfg = fopen(NDLESS_CONFIG, "a");
			fprintf(ndlesscfg, "\next.%s=%s\n", extension, name);
		}
		fclose(ndlesscfg);
		free(whole_file);
	}
	else
	{	ndlesscfg = fopen(NDLESS_CONFIG, "w");
		fprintf(ndlesscfg, "ext.%s=%s\n", extension, name);
		fclose(ndlesscfg);
	}
}

//
// D_DoomMain
//
extern int mode_bits;
unsigned char sscreen[SCREEN_SIZE];
void D_DoomMain (int argc, char* argv[])
{	int i;
	addshellext("wad","ndoom");
	char* loc=argv[0];
	//Find the current directory
	I_GetMainDir(loc);
	int ok_wad=0;
	int selected=1;
	strcpy(doomwad,current_dir);
	
	startScreen();
	convertRGB565(sscreen, SCR_320x240_565);
	while(!ok_wad)
	{	char* t =doomwad+strlen(doomwad)-1;
		while(*t!='/') t--;
		*(t+1)=0;
		if(argc>1)
		{	strcpy(doomwad,argv[1]);
			selected=1;
			argc=1;
		}
		else
			selected=chooseFile(doomwad,doomwad,"Choose Doom/Doom2 compatible IWAD file:");
		if(!selected) return;
		D_AddFile (doomwad);
		ok_wad=W_InitMultipleFiles (wadfiles);
	}
	if(W_CheckNumForName("MAP01")!=-1)
		gamemode=commercial;
	else if(W_CheckNumForName("E1M1")==-1)
		gamemode=indetermined;
	else if (W_CheckNumForName("E2M1")==-1)
		gamemode=shareware;
	else if (W_CheckNumForName("E4M1")==-1)
		gamemode=registered;
	else
		gamemode=retail;		

    if(gamemode == indetermined) // CX port
	return; // CX port

	char* doomwadf = doomwad+strlen(doomwad)-1; // CX port
	while(*doomwadf!='/') doomwadf--; // CX port
	strcpy(doomsav,doomwadf); // CX port
	doomsav[strlen(doomsav)-7]='s'; // CX port
	doomsav[strlen(doomsav)-5]='v'; // CX port

	char res[6];
	int maxepisode=1;
	while(true)
	{	sprintf(res,"E%iM1",maxepisode+1);
		if (W_CheckNumForName(res)==-1)
			break;
		maxepisode++;
	}
	int maxmap=1;
	while(true)
	{	if(gamemode==commercial)
			sprintf(res,"MAP%02i",maxmap+1);
		else
			sprintf(res,"E1M%i",maxmap+1);
		if (W_CheckNumForName(res)==-1)
			break;
		maxmap++;
	}

	int rskill=1;
	menu(maxepisode);
	int refresh=0;
/*	I_DispHeader();
	drwBufStr(getScreen(), 0, 11*CHAR_HEIGHT, "Skill: [-]                      [+]", 0, 1);
	drwBufStr(getScreen(), 0, 18*CHAR_HEIGHT, "    For guns and glory press Enter!", 0, 1);
	drwBufStr(getScreen(), 0, 19*CHAR_HEIGHT, "    For work and worry press Esc...", 0, 1);
	if(maxepisode>1)
		drwBufStr(getScreen(), 0,  15*CHAR_HEIGHT, "Episode: [(]   [)]", 0, 1);
*/
	int repisode=1;
	int rmap=1;
	char temp[3];
	char* tmp;
	char* tmpwad;
	char* tmpwad2;
//	strcpy(tmpwad,doomwad);
	do
	{	if(isKeyPressed(KEY_NSPIRE_TAB))
		{	tmpwad2=malloc(512);
			strcpy(tmpwad2,doomwad);
			tmpwad=tmpwad2;
			char* t =tmpwad+strlen(tmpwad)-1;
			while(*t!='/') t--;
			*(t+1)=0;
			selected=chooseFile(tmpwad,tmpwad,"Choose Doom/Doom2 compatible PWAD file:");
			if(selected)
			{	D_AddFile(tmpwad);
				ok_wad = W_InitMultipleFiles(wadfiles);
				if(!ok_wad)
				{	nwad--;
					free(tmpwad);
					wadfiles[nwad]=0;
				}
			}
			menu(maxepisode);
			refresh=1;
			while(any_key_pressed());
		}
		if(maxepisode>1)
		{	if(isKeyPressed(KEY_NSPIRE_RP) && episode<maxepisode)	{	episode++; repisode=1; while(isKeyPressed(KEY_NSPIRE_RP)); }
			if(isKeyPressed(KEY_NSPIRE_LP) && episode>1)		{	episode--; repisode=1; while(isKeyPressed(KEY_NSPIRE_LP)); }
			if(repisode || refresh)
			{	sprintf(temp,"%02i",episode);
				drwBufStr(getScreen(), 13*CHAR_WIDTH,  15*CHAR_HEIGHT, temp, 0, 0);
				repisode = 0;
			}
		}
		if(isKeyPressed(KEY_NSPIRE_C) && map>1)		{	map--; rmap=1; while(isKeyPressed(KEY_NSPIRE_C)); }
		if(isKeyPressed(KEY_NSPIRE_D) && map<maxmap)	{	map++; rmap=1; while(isKeyPressed(KEY_NSPIRE_D)); }
		if(rmap || refresh)
		{	sprintf(temp,"%02i",map);
			drwBufStr(getScreen(), 13*CHAR_WIDTH,  14*CHAR_HEIGHT, temp, 0, 0);
			rmap = 0;
		}
		if(isKeyPressed(KEY_NSPIRE_PLUS) && skill<4)	{	skill++; rskill=1; while(isKeyPressed(KEY_NSPIRE_PLUS)); }
		if(isKeyPressed(KEY_NSPIRE_MINUS) && skill>0)		{	skill--; rskill=1; while(isKeyPressed(KEY_NSPIRE_MINUS)); }
		if(rskill || refresh)
		{	if(skill==0)	tmp="I'm too young to die";
			else if(skill==1)	tmp="  Hey, not to rough ";
			else if(skill==2)	tmp="   Hurt me plenty   ";
			else if(skill==3)	tmp="   Ultra-Violence   ";
			else if(skill==4)	tmp="     Nightmare!     ";
			drwBufStr(getScreen(), 13*CHAR_WIDTH, 13*CHAR_HEIGHT, tmp, 0, 0);
			rskill = 0;
		}
		refresh=0;
	}
	while(!isKeyPressed(KEY_NSPIRE_ESC) && !isKeyPressed(KEY_NSPIRE_ENTER) && !isKeyPressed(KEY_NSPIRE_RET) && !isKeyPressed(KEY_NSPIRE_CLICK));


    if(isKeyPressed(KEY_NSPIRE_ESC)) // CX port
	return; // CX port

    modifiedgame = false;
	
    nomonsters = 0;
    respawnparm = 0;
    fastparm = 0;
    devparm = 0;

    switch ( gamemode )
    {
      case retail:
		sprintf (title,
			 "The Ultimate DOOM Startup v%i.%i",
			 VERSION/100,VERSION%100);
		break;
      case shareware:
		sprintf (title,
			 "DOOM Shareware Startup v%i.%i",
			 VERSION/100,VERSION%100);
		break;
      case registered:
		sprintf (title,
			 "DOOM Registered Startup v%i.%i",
			 VERSION/100,VERSION%100);
		break;
      case commercial:
		sprintf (title,
			 "DOOM 2: Hell on Earth v%i.%i",
			 VERSION/100,VERSION%100);
		break;
    
      default:
		sprintf (title,
			 "Public DOOM - v%i.%i",
			 VERSION/100,VERSION%100);
		break;
    }
	
    printf ("%s\n",title);
	
    // get skill / episode / map from parms // DON'T, LEFT IN UNTIL OPCHECK
    if(skill==0)		startskill = sk_baby;
    else if(skill==1)	startskill = sk_easy;
    else if(skill==2)	startskill = sk_medium;
    else if(skill==3)	startskill = sk_hard;
    else if(skill==4)	startskill = sk_nightmare;

    startepisode = episode;
    startmap = map;

//    G_DeferedInitNew(2,1,1); // CX port
    autostart = false;
	
    // init subsystems
    printf ("V_Init: Allocate screens\n");
    V_Init ();
    printf ("Z_Init: Init zone memory allocation daemon \n");
    Z_Init ();
    printf ("W_Init: Init WADfiles\n");

	printf("Running from %s \n",current_dir);
	
    W_InitMultipleFiles (wadfiles);
    for(i=1;i<nwad;i++)
	free(wadfiles[i]);

    //Just in case we haven't found the WAD files...
	if (fuck)
		return;

    // Check for -file in shareware
    if (modifiedgame)
    {
		// These are the lumps that will be checked in IWAD,
		// if any one is not present, execution will be aborted.
		char name[23][8]=
		{
			"e2m1","e2m2","e2m3","e2m4","e2m5","e2m6","e2m7","e2m8","e2m9",
			"e3m1","e3m3","e3m3","e3m4","e3m5","e3m6","e3m7","e3m8","e3m9",
			"dphoof","bfgga0","heada1","cybra1","spida1d1"
		};
		int i;
		
		if ( gamemode == shareware)
			I_Error("You cannot -file with the shareware "
				"version. Register!");

		// Check for fake IWAD with right name,
		// but w/o all the lumps of the registered version. 
		if (gamemode == registered)
			for (i = 0;i < 23; i++)
				if (W_CheckNumForName(name[i])<0)
					I_Error("This is not the registered version.");
    }
    // Check and print which version is executed.
    switch ( gamemode )
    {
      case shareware:
      case indetermined:
		printf (
			"=====================================\n"
			"             Shareware!\n"
			"=====================================\n"
		);
		break;
      case registered:
      case retail:
      case commercial:
		printf (
			"=====================================\n"
			"          Commercial product\n"
			"          do not distribute!\n"
			"=====================================\n"
		);
		break;
	
      default:
		// Ouch.
		break;
    }

	printf ("I_Init: Setting up machine state\n");
    I_Init ();
    printf ("M_Init: Init miscellaneous info\n");
    M_Init ();
    printf ("R_Init: Init DOOM refresh daemon \n");
    R_Init ();
    printf ("P_Init: Init Playloop state\n");
    P_Init ();
    printf ("HU_Init: Setup heads up display\n");
    HU_Init ();
    printf ("ST_Init: Init status bar\n");
    ST_Init ();
    initTP(); // CX port
	printf ("Engage... \n");
	D_StartTitle ();
	stopScreen();
    
	D_DoomLoop ();  // never returns, except when it does
	I_Restore (); //calckill = bad
	
	return;
}
