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
/**@name map_rock.c	-	The map rock handling. */
/*
**	(c) Copyright 1999,2000 by Vladi
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"
#include "map.h"
#include "minimap.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

 // FIXME: -1 is hack should be fixed later
#define FIRST_ROCK_TILE  (TheMap.Tileset->FirstRockTile-1)

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local const int RockTable[16] = {
//  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  A,  B,  C,  D,  E,  F
   -1, 22, -1,  1, 20, 21,  3,  2, -1,  9, -1, 23,  6,  8,  5, 36
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Check if the tile type is rock.
**
** 	Used by @see FixRock and @see PreprocessMap
*/
local int MapRockChk(int x,int y)
{
    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	return 1;		// outside considered rock
    }

    return TheMap.Tileset->TileTypeTable[
	    TheMap.Fields[(x)+(y)*TheMap.Width].SeenTile
	] == TileTypeRock;
}

// FIXME: docu
local int FixRock(int x,int y) // used by MapRemoveRock and PreprocessMap
{
    int tile;
    MapField* mf;

    //	Outside map or no rock.
    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	return 0;
    }
    if ( !MapRockChk(x,y) ) {
	return 0;
    }

#define ROCK(xx,yy) (MapRockChk(xx,yy) != 0)
    tile = 0;
    if (ROCK(x  ,y-1)) tile |= 1<<0;
    if (ROCK(x+1,y  )) tile |= 1<<1;
    if (ROCK(x  ,y+1)) tile |= 1<<2;
    if (ROCK(x-1,y  )) tile |= 1<<3;

    tile = RockTable[tile];
    if (tile == -1) {
	MapRemoveRock(x,y);
    } else {
	if (tile == RockTable[15]) {
    // Vladi: still to filter tiles w. corner empties -- note: the original
    // tiles and order are not perfect either. It's a hack but is enough and
    // looks almost fine.
	    if (MapRockChk(x+1,y-1) == 0) tile =  7; else
	    if (MapRockChk(x+1,y+1) == 0) tile = 10; else
	    if (MapRockChk(x-1,y+1) == 0) tile = 11; else
	    if (MapRockChk(x-1,y-1) == 0) tile =  4; else
				tile = RockTable[15]; // not required really
	}

	tile += FIRST_ROCK_TILE;
	mf=TheMap.Fields+x+y*TheMap.Width;

	if ( mf->SeenTile == tile) {
	    return 0;
	}
	mf->SeenTile =  tile;
    }
    UpdateMinimapXY(x,y);
    return 1;
}

// FIXME: docu
global void MapFixRock(int x,int y)
{
    // side neighbors
    FixRock( x+1, y   );
    FixRock( x-1, y   );
    FixRock( x  , y+1 );
    FixRock( x  , y-1 );
}

/**
**	Remove rock from the map.
**
**	@param x	Map X position.
**	@param y	Map Y position.
*/
global void MapRemoveRock(unsigned x,unsigned y)
{
    MapField* mf;

    mf=TheMap.Fields+x+y*TheMap.Width;

    mf->Tile=TheMap.Tileset->NoRockTile;
    mf->Flags &= ~(MapFieldRocks|MapFieldUnpassable);

    UpdateMinimapXY(x,y);		// FIXME: should be done if visible?

    // Must redraw map only if field is visibile
    if( mf->Flags&MapFieldVisible ) {
	MustRedraw|=RedrawMaps;
    }
}

//@}
