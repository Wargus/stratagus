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
/**@name construct.c	-	The constructions. */
//
//	(c) Copyright 1998-2000 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "construct.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifndef laterUSE_CCL
// FIXME: Make this configurable with CCL and than use only this.

#define DEFAULT	NULL			/// use the first slot as default

/**
**	Constructions.
*/
local Construction Constructions[] = {
{ "construction-0",			// added for bad puds they have 0
#ifdef NEW_NAMES
  { "neutral/buildings/land construction site.png",
  "tilesets/winter/neutral/buildings/land construction site.png",
  DEFAULT,
  DEFAULT },
#else
  { "land construction site (summer,wasteland).png",
  "land construction site (winter).png",
  DEFAULT,
  DEFAULT },
#endif
	64,64 },
{ "construction-1",
  { "",
  DEFAULT,
  DEFAULT,
  DEFAULT },
	0,0 },
{ "construction-2",
  { "",
  DEFAULT,
  DEFAULT,
  DEFAULT },
	0,0 },
{ "construction-3",
  { "",
  DEFAULT,
  DEFAULT,
  DEFAULT },
	0,0 },
{ "construction-4",
  { "",
  DEFAULT,
  DEFAULT,
  DEFAULT },
	0,0 },
{ "construction-5",
  { "",
  DEFAULT,
  DEFAULT,
  DEFAULT },
	0,0 },
{ "construction-land",
#ifdef NEW_NAMES
  { "neutral/buildings/land construction site.png",
  "tilesets/winter/neutral/buildings/land construction site.png",
  DEFAULT,
  DEFAULT },
#else
  { "land construction site (summer,wasteland).png",
  "land construction site (winter).png",
  DEFAULT,
  DEFAULT },
#endif
	64,64 },
{ "construction-human-shipyard",
#ifdef NEW_NAMES
  { "human/buildings/shipyard construction site.png",
  "tilesets/winter/human/buildings/shipyard construction site.png",
  DEFAULT,
  "tilesets/swamp/human/buildings/shipyard construction site.png" },
#else
  { "human shipyard construction site (summer,wasteland).png",
  "human shipyard construction site (winter).png",
  DEFAULT,
  "human shipyard construction site (swamp).png" },
#endif
	96,96 },
{ "construction-orc-shipyard",
#ifdef NEW_NAMES
  { "orc/buildings/shipyard construction site.png",
  "tilesets/winter/orc/buildings/shipyard construction site.png",
  DEFAULT,
  "tilesets/swamp/orc/buildings/shipyard construction site.png" },
#else
  { "orc shipyard construction site (summer,wasteland).png",
  "orc shipyard construction site (winter).png",
  DEFAULT,
  "orc shipyard construction site (swamp).png" },
#endif
	96,96 },
{ "construction-human-oil-well",
#ifdef NEW_NAMES
  { "tilesets/summer/human/buildings/oil well construction site.png",
  "tilesets/winter/human/buildings/oil well construction site.png",
  "tilesets/wasteland/human/buildings/oil well construction site.png",
  "tilesets/swamp/human/buildings/oil well construction site.png" },
#else
  { "human oil well construction site (summer).png",
  "human oil well construction site (winter).png",
  "human oil well construction site (wasteland).png",
  "human oil well construction site (swamp).png" },
#endif
	96,96 },
{ "construction-orc-oil-well",
#ifdef NEW_NAMES
  { "tilesets/summer/orc/buildings/oil well construction site.png",
  "tilesets/winter/orc/buildings/oil well construction site.png",
  "tilesets/wasteland/orc/buildings/oil well construction site.png",
  "tilesets/swamp/orc/buildings/oil well construction site.png" },
#else
  { "orc oil well construction site (summer).png",
  "orc oil well construction site (winter).png",
  "orc oil well construction site (wasteland).png",
  "orc oil well construction site (swamp).png" },
#endif
	96,96 },
{ "construction-human-refinery",
#ifdef NEW_NAMES
  { "human/buildings/refinery construction site.png",
  "tilesets/winter/human/buildings/refinery construction site.png",
  DEFAULT,
  "tilesets/swamp/human/buildings/refinery construction site.png" },
#else
  { "human refinery construction site (summer,wasteland).png",
  "human refinery construction site (winter).png",
  DEFAULT,
  "human refinery construction site (swamp).png" },
#endif
	96,96 },
{ "construction-orc-refinery",
#ifdef NEW_NAMES
  { "orc/buildings/refinery construction site.png",
  "tilesets/winter/orc/buildings/refinery construction site.png",
  DEFAULT,
  "tilesets/swamp/orc/buildings/refinery construction site.png" },
#else
  { "orc refinery construction site (summer,wasteland).png",
  "orc refinery construction site (winter).png",
  DEFAULT,
  "orc refinery construction site (swamp).png" },
#endif
	96,96 },
{ "construction-human-foundry",
#ifdef NEW_NAMES
  { "human/buildings/foundry construction site.png",
  "tilesets/winter/human/buildings/foundry construction site.png",
  DEFAULT,
  "tilesets/swamp/human/buildings/foundry construction site.png" },
#else
  { "human foundry construction site (summer,wasteland).png",
  "human foundry construction site (winter).png",
  DEFAULT,
  "human foundry construction site (swamp).png" },
#endif
	96,96 },
{ "construction-orc-foundry",
#ifdef NEW_NAMES
  { "orc/buildings/foundry construction site.png",
  "tilesets/winter/orc/buildings/foundry construction site.png",
  DEFAULT,
  "tilesets/swamp/orc/buildings/foundry construction site.png" },
#else
  { "orc foundry construction site (summer,wasteland).png",
  "orc foundry construction site (winter).png",
  DEFAULT,
  "orc foundry construction site (swamp).png" },
#endif
	96,96 },
{ "construction-wall",
#ifdef NEW_NAMES
  { "tilesets/summer/neutral/buildings/wall construction site.png",
  "tilesets/winter/neutral/buildings/wall construction site.png",
  "tilesets/wasteland/neutral/buildings/wall construction site.png",
  DEFAULT },
#else
  { "wall construction site (summer).png",
  "wall construction site (winter).png",
  "wall construction site (wasteland).png",
  DEFAULT },
#endif
	32,32 },
};

#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Load the graphics for the constructions.
**
**	HELPME:	who make this better terrain depended and extendable
**	HELPME: filename constuction.
*/
global void LoadConstructions(void)
{
    int i;
    const char* file;

    for( i=0; i<sizeof(Constructions)/sizeof(*Constructions); ++i ) {
	file=Constructions[i].File[TheMap.Terrain];
	if( !file ) {			// default one
	    file=Constructions[i].File[0];
	}
	if( *file ) {
	    char* buf;

	    buf=alloca(strlen(file)+9+1);
#ifdef NEW_NAMES
	    file=strcat(strcpy(buf,"graphics/"),file);
#else
	    file=strcat(strcpy(buf,"graphic/"),file);
#endif
	    ShowLoadProgress("\tConstruction %s\n",file);
	    Constructions[i].Sprite=LoadSprite(file
		    ,Constructions[i].Width,Constructions[i].Height);
	}
    }
}

/**
**	Draw construction.
**
**	@param type	Type number of construction.
**	@param frame	Frame number to draw.
**	@param x	X position.
**	@param y	Y position.
*/
global void DrawConstruction(int type,int frame,int x,int y)
{
    // FIXME: This should be moved to caller/init...
    x-=Constructions[type].Width/2;
    y-=Constructions[type].Height/2;

    VideoDrawClip(Constructions[type].Sprite,frame,x,y);
}

//@}
