//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name image.c	-	The standard images. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

//@{

// FIXME: seems that this part could and should be removed.

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "image.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Constant graphics
*/
struct _images_ {
    char*	File[PlayerMaxRaces];	/// one file for each race
    unsigned	Width;			/// graphic size width
    unsigned	Height;			/// graphic size height

// --- FILLED UP ---
    Graphic*	Image;			/// graphic image (filled)
} Images[] = {
    { { "interface/panel 1 (humans).png"
	,"interface/panel 1 (orcs).png" }
		, 256, 288 },
    { { "interface/panel 2 (humans).png"
	,"interface/panel 2 (orcs).png" }
		, 288, 256 },
    { { "interface/panel 3 (humans).png"
	,"interface/panel 3 (orcs).png" }
		, 384, 256 },
    { { "interface/panel 4 (humans).png"
	,"interface/panel 4 (orcs).png" }
		, 288, 128 },
    { { "interface/panel 5 (humans).png"
	,"interface/panel 5 (orcs).png" }
		, 352, 352 },
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Draw image on screen.
**
**	@param image	Image number (=index).
**	@param row	Image row
**	@param frame	Image frame
**	@param X	X position.
**	@param Y	Y position.
*/
global void DrawImage(int image,int row,int frame,int x,int y)
{
    if( image>=0 ) {			// FIXME: trick 17! better solution
	VideoDrawSub(Images[image].Image
	    ,frame*Images[image].Width,row*Images[image].Height
	    ,Images[image].Width,Images[image].Height
	    ,x,y);
    }
}

/**
**	Load all images.
*/
global void LoadImages(unsigned int race)
{
    int i;
    const char* file;
    char* buf;
    static int last_race = -1;

    if (race == last_race)	// same race? already loaded!
	return;
    if (last_race != -1) {	// free previous images for different race
	for( i=0; i<sizeof(Images)/sizeof(*Images); ++i ) {
	    VideoSaveFree(Images[i].Image);
	}
    }
    last_race = race;

    //
    //	Load all images, depends on the race of the player on this computer.
    //
    for( i=0; i<sizeof(Images)/sizeof(*Images); ++i ) {
	file=Images[i].File[race];
	if( !file ) {			// default one
	    file=Images[i].File[0];
	}

	buf=alloca(strlen(file)+9+1);
	file=strcat(strcpy(buf,"graphic/"),file);
	ShowLoadProgress("\tImage %s\n",file);
	Images[i].Image=LoadGraphic(file);
    }
}

/**
**	Return image width
**
**	@param image	Image number (=index).
**
**	@returns	The image width.
*/
global int ImageWidth(int image)
{
    return Images[image].Width;
}

/**
**	Return image height
**
**	@param image	Image number (=index).
**
**	@returns	The image height.
*/
global int ImageHeight(int image)
{
    return Images[image].Height;
}

//@}
