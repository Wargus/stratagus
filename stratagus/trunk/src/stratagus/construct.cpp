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

#define DEFAULT	NULL			/// use the first slot as default

/**
**	Constructions.
*/
local Construction Constructions[] = {
{ "construction-0",			// added for bad puds they have 0
  { "land construction site (summer,wasteland).png",
  "land construction site (winter).png",
  DEFAULT,
  DEFAULT },
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
  { "land construction site (summer,wasteland).png",
  "land construction site (winter).png",
  DEFAULT,
  DEFAULT },
	64,64 },
{ "construction-human-shipyard",
  { "human shipyard construction site (summer,wasteland).png",
  "human shipyard construction site (winter).png",
  DEFAULT,
  "human shipyard construction site (swamp).png" },
	96,96 },
{ "construction-orc-shipyard",
  { "orc shipyard construction site (summer,wasteland).png",
  "orc shipyard construction site (winter).png",
  DEFAULT,
  "orc shipyard construction site (swamp).png" },
	96,96 },
{ "construction-human-oil-well",
  { "human oil well construction site (summer).png",
  "human oil well construction site (winter).png",
  "human oil well construction site (wasteland).png",
  "human oil well construction site (swamp).png" },
	96,96 },
{ "construction-orc-oil-well",
  { "orc oil well construction site (summer).png",
  "orc oil well construction site (winter).png",
  "orc oil well construction site (wasteland).png",
  "orc oil well construction site (swamp).png" },
	96,96 },
{ "construction-human-refinery",
  { "human refinery construction site (summer,wasteland).png",
  "human refinery construction site (winter).png",
  DEFAULT,
  "human refinery construction site (swamp).png" },
	96,96 },
{ "construction-orc-refinery",
  { "orc refinery construction site (summer,wasteland).png",
  "orc refinery construction site (winter).png",
  DEFAULT,
  "orc refinery construction site (swamp).png" },
	96,96 },
{ "construction-human-foundry",
  { "human foundry construction site (summer,wasteland).png",
  "human foundry construction site (winter).png",
  DEFAULT,
  "human foundry construction site (swamp).png" },
	96,96 },
{ "construction-orc-foundry",
  { "orc foundry construction site (summer,wasteland).png",
  "orc foundry construction site (winter).png",
  DEFAULT,
  "orc foundry construction site (swamp).png" },
	96,96 },
{ "construction-wall",
  { "wall construction site (summer).png",
  "wall construction site (winter).png",
  "wall construction site (wasteland).png",
  DEFAULT },
	32,32 },
};

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
	    file=strcat(strcpy(buf,"graphic/"),file);
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
