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
//	Handles WAD file header, directory, lump I/O.
//
//-----------------------------------------------------------------------------


//static const char 

//rcsid[] = "$Id: w_wad.c,v 1.5 1997/02/03 16:47:57 b1 Exp $";


/*#ifdef NORMALUNIX
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <alloca.h>
#define O_BINARY		0
#endif*/

#include <os.h>

#include "doomtype.h"
#include "m_swap.h"
#include "i_system.h"
#include "z_zone.h"

#ifdef __GNUG__
#pragma implementation "w_wad.h"
#endif
#include "w_wad.h"




//
// GLOBALS
//

// Location of each lump on disk.
lumpinfo_t*		lumpinfo;
int				numlumps=0;

void**			lumpcache;


/*void strupr (char* s)
{
    while (*s) { *s = toupper_int(*s); s++; }
}*/

int filelength (char *filename) 
{ 
    struct stat	fileinfo;
	
	printf ("filelength(%s) called \n",filename);
    
	//AAAAARGH, FSTAT/STAT, ALL DIFFERENT, FUCK THIS SHIT!!!!
    if (stat (filename,&fileinfo) == -1)
		I_Error ("Error stat()ing");

	printf ("Returning %i \n",fileinfo.st_size);
	
    return fileinfo.st_size;
}


void ExtractFileBase ( char* path, char* dest )
{
    char*	src;
    int		length;

    src = path + strlen(path) - 1;
    
    // back up until a \ or the start
    while (src != path && *(src-1) != '\\' && *(src-1) != '/')
    {
		src--;
    }
    
    // copy up to eight characters
    memset (dest,0,8);
    length = 0;
    
    while (*src && *src != '.')
    {
		if (++length == 9)
			I_Error ("Filename base of %s >8 chars",path);

		*dest++ = toupper_int((int)*src++);
    }
}


//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// If filename starts with a tilde, the file is handled
//  specially to allow map reloads.
// But: the reload feature is a fragile hack...
// THAT'S WHY RELOADABLE FILE SUPPORT WAS REMOVED, YOU FUCKERS! STOP USING CRAPPY HACKS!

//int			reloadlump;
//char*			reloadname;


int W_AddFile (char *filename) // CX port
{
    wadinfo_t		header;
    lumpinfo_t*		lump_p;
    //unsigned		i;
	int				i;
    FILE*			handle;
    int				length;
    int				startlump;
    filelump_t*		fileinfo;
	filelump_t*		fileinfo_mem;
    //filelump_t		singleinfo;
	
    // open the file and add to directory

    /*// handle reload indicator.
    if (filename[0] == '~')
    {
		filename++;
		reloadname = filename;
		reloadlump = numlumps;
    }*/
	
	handle = fopen (filename,"rb");
//printf("%i",(int)(handle));
    if ( handle==NULL )
    {
		printf ("Couldn't open %s\n",filename);
		return 0; // CX port
    }

    //printf ("Adding %s\n",filename);
    startlump = numlumps;
	
	/*//NSPIRE NOTE: Make sure 'filename' is ALL CAPS, this comparison is supposed to be case-ignoring, but nspire-gcc lacks that function
    if (strcmp (filename+strlen(filename)-3 , "WAD" ) )
    {
		// single lump file
		fileinfo = &singleinfo;
		singleinfo.filepos = 0;
		singleinfo.size = LONG(filelength(filename));
		ExtractFileBase (filename, singleinfo.name);
		numlumps++;
    }
    else 
	{*/
		// WAD file
		//read (handle, &header, sizeof(header));
		//fread ( dest, size elements, count elements, FILE handle );
		fread (&header, sizeof(header), 1, handle);
		if (strncmp(header.identification,"IWAD",4))
		{
			// Homebrew levels?
			if (strncmp(header.identification,"PWAD",4))
			{	fclose(handle); // CX port
				I_Error ("Wad file %s doesn't have IWAD or PWAD id\n", filename);
				return 0; // CX port
			}
		}
		header.numlumps = LONG(header.numlumps);
		header.infotableofs = LONG(header.infotableofs);
		length = header.numlumps * sizeof(filelump_t);
		//fileinfo = alloca (length);
		fileinfo_mem= malloc(length);
		fileinfo= fileinfo_mem;
		//printf ("M:length= %i \n",length);
		//lseek (handle, header.infotableofs, SEEK_SET);
		//fseek ( FILE handle, offset, origin of seek );
		fseek (handle, header.infotableofs, SEEK_SET);
		//read (handle, fileinfo, length);
		//fread ( dest, size elements, count elements, FILE handle );
		fread (fileinfo, length, 1, handle);
		numlumps += header.numlumps;
    //}

    
    // Fill in lumpinfo
    lumpinfo = realloc (lumpinfo, numlumps*sizeof(lumpinfo_t));

    if (!lumpinfo)
		I_Error ("Couldn't realloc lumpinfo");

    lump_p = &lumpinfo[startlump];
	
    for (i=startlump ; i<numlumps ; i++,lump_p++, fileinfo++)
    {
		lump_p->handle = handle; //Originally, this was storehandle... hopefully this won't break anything...
		lump_p->position = LONG(fileinfo->filepos);
		lump_p->size = LONG(fileinfo->size);
		strncpy (lump_p->name, fileinfo->name, 8);
    }
	
	//Nspire has no alloca() function for temporary memory, so we have to use malloc() and free() to compensate
	free(fileinfo_mem);
	return 1; // CX port
}

//
// W_InitMultipleFiles
// Pass a null terminated list of files to use.
// All files are optional, but at least one file
//  must be found.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//

int W_InitMultipleFiles (char** filenames) // CX port
{   int		size;
    int i;
    if(numlumps)
    {	lumpinfo_t*		lump_p;
	lump_p = &lumpinfo[0];
	for (i=0 ; i<numlumps ; i++,lump_p++)
	{	fclose(lump_p->handle);
//printf("%i",-(int)(lump_p->handle));
	}
	free(lumpinfo);
	free(lumpcache);
    }
    
    // open all the files, load headers, and count lumps
    numlumps = 0;

    // will be realloced as lumps are added
    lumpinfo = malloc(1);	

    for ( ; *filenames ; filenames++)
	{	//displnBuf(SCREEN_BASE_ADDRESS,*filenames);
		if(!W_AddFile (*filenames))
		{	free(lumpinfo); // CX port
			return 0; // CX port
		}
	}

    if (!numlumps)
		I_Error ("W_InitFiles: no files found");
    
    // set up caching
	//printf ("numlumps = %i \n",numlumps);
    size = numlumps * sizeof(*lumpcache);
    lumpcache = malloc (size);
    
    if (!lumpcache)
		I_Error ("Couldn't allocate lumpcache");

    memset (lumpcache,0, size);
    return 1; // CX port
}




//
// W_InitFile
// Just initialize from a single file.
//
void W_InitFile (char* filename)
{
    char*	names[2];

    names[0] = filename;
    names[1] = NULL;
    W_InitMultipleFiles (names);
}



//
// W_NumLumps
//
int W_NumLumps (void)
{
    return numlumps;
}



//
// W_CheckNumForName
// Returns -1 if name not found.
//

int W_CheckNumForName (char* name)
{
    /*union {
		char	s[9];
		int		x[2];
    } name8;
    
    int		v1;
    int		v2;*/
    lumpinfo_t*	lump_p;
	char*		temp= "        ";

	/*
    // make the name into two integers for easy compares
    strncpy (name8.s,name,8);

    // case insensitive
    //strupr (name);

	// in case the name was a fill 8 chars
    name8.s[8] = 0; //???? 
	
    v1 = name8.x[0];
    v2 = name8.x[1];

	printf ("name= %s \n",name);
	printf ("name8= %s \n",name8.s);
	printf ("v1= %i \n",v1);
	printf ("v2= %i \n",v2);*/
	strupr (name);

    // scan backwards so patch lump files take precedence
    lump_p = lumpinfo + numlumps;
	
    while (lump_p-- != lumpinfo)
    {
		strncpy (temp,lump_p->name,8);
		
		/*if ( *(int *)lump_p->name == v1    &&    *(int *)&lump_p->name[4] == v2) <-CAN'T FIND SHIT, FOR SOME REASON
			return lump_p - lumpinfo;*/
		
		//strupr (lump_p->name);
		//if (strcmp(name,lump_p->name)==0 )
		if (strcmp(name,temp)==0 )
		{
			/*if (strcmp(name,"SKY1")==0 )
				printf("skylump = %i \n",lump_p - lumpinfo);*/
			/*if (strcmp(name,"F_END")==0)
				printf("( %s/%s %i) ",name, temp, lump_p - lumpinfo);*/
			//printf("w_checknumforname(%s) returning %i\n", name, lump_p - lumpinfo);
			return lump_p - lumpinfo;
		}
		
    }
	
    // TFB. Not found.
    return -1;
}




//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName (char* name)
{
    int	i;
	//printf("w_getnumforname(%s) called \n",name);
    i = W_CheckNumForName (name);
    
    if (i == -1)
	{
		printf("Accessing %s \n",name);
		I_Error ("W_GetNumForName: %s not found!", name);
    }
	
    return i;
}


//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength (int lump)
{
    if (lump >= numlumps)
		I_Error ("W_LumpLength: %i >= numlumps",lump);

    return lumpinfo[lump].size;
}



//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump ( int lump, void* dest )
{
    int				c;
    lumpinfo_t*		l;
    FILE*			handle;
	
    if (lump >= numlumps)
		I_Error ("W_ReadLump: %i >= numlumps",lump);

    l = lumpinfo+lump;
	
    /*if (l->handle == -1)   WE ARE NSPIRED, WE HAVE NO NEED FOR RELOADABLE FILES
    {
		// reloadable file, so use open / read / close
		if ( (handle = open (reloadname,O_RDONLY | O_BINARY)) == -1)
			I_Error ("W_ReadLump: couldn't open %s",reloadname);
    }
    else
		handle = l->handle;*/
	handle = l->handle;
	
	//Jesus Fick, you'd think the difference between fseek() and lseek() would be SLIGHTLY more than just the name of the function
	//lseek (handle, l->position, SEEK_SET);
    fseek (handle, l->position, SEEK_SET);
	
	
    //c = read (handle, dest, l->size);
	//fread ( dest, size elements, count elements, FILE handle );
	c= fread (dest, 1, l->size, handle);
	
	/*printf("W_ReadLump(%i,...) in progress... \n",lump);
	printf("l->handle = %i \n",(int)l->handle);
	printf("l->size = %i \n",l->size);
	printf("l->position = %i \n",l->position);
	printf("c= %i \n",c);*/

    if (c < l->size)
		I_Error ("W_ReadLump: only read %i of %i on lump %i", c,l->size,lump);	

    /*if (l->handle == -1) WE HAVE NO NEED FOR RELOADABLE FILES
		fclose (handle);*/
}




//
// W_CacheLumpNum
//
void* W_CacheLumpNum ( int lump, int tag )
{
    byte*	ptr;
	
    if (lump >= numlumps)
		I_Error ("W_CacheLumpNum: %i >= numlumps (%i)",lump,numlumps);
	
	
	//printf("x");
	
    if (!lumpcache[lump])
    {
		// read the lump in
		
		//printf ("cache miss on lump %i\n",lump);
		ptr = Z_Malloc (W_LumpLength (lump), tag, &lumpcache[lump]);
		W_ReadLump (lump, lumpcache[lump]);
    }
    else
    {
		//printf ("cache hit on lump %i\n",lump);
		Z_ChangeTag (lumpcache[lump],tag);
    }
	
	/*if (lump==1203)
		printf("sky access, returning %x \n",lumpcache[lump]);*/
	
    return lumpcache[lump];
}

//
// W_CacheLumpName
//
void* W_CacheLumpName ( char* name, int tag )
{
	int i;
	
	i= W_GetNumForName(name);
	
	return W_CacheLumpNum (i, tag);
    //return W_CacheLumpNum (W_GetNumForName(name), tag);
}



/*    WE ARE THE NSPIRED. WE HAVE NO NEED FOR RELOADABLE MAPS. WE COMMENT THIS FUNCTION.
//
// W_Reload
// Flushes any of the reloadable lumps in memory
//  and reloads the directory.
//
void W_Reload (void)
{
    wadinfo_t		header;
    int				lumpcount;
    lumpinfo_t*		lump_p;
    unsigned		i;
    FILE*			handle;
    int				length;
    filelump_t*		fileinfo;
	
    if (!reloadname)
		return;
		
    if ( (handle = open (reloadname,O_RDONLY | O_BINARY)) == -1)
		I_Error ("W_Reload: couldn't open %s",reloadname);

    read (handle, &header, sizeof(header));
    lumpcount = LONG(header.numlumps);
    header.infotableofs = LONG(header.infotableofs);
    length = lumpcount*sizeof(filelump_t);
    fileinfo = alloca (length);
    lseek (handle, header.infotableofs, SEEK_SET);
    read (handle, fileinfo, length);
    
    // Fill in lumpinfo
    lump_p = &lumpinfo[reloadlump];
	
    for (i=reloadlump ;
	 i<reloadlump+lumpcount ;
	 i++,lump_p++, fileinfo++)
    {
	if (lumpcache[i])
	    Z_Free (lumpcache[i]);

	lump_p->position = LONG(fileinfo->filepos);
	lump_p->size = LONG(fileinfo->size);
    }
	
    close (handle);
}
*/