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
//
//	(c) Copyright 1999-2001 by Vladi Shabanski and Lutz Sammer
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

/*
**	Note:
**		This functions are doubled. One for the real map tile and one
**		for the tile that the player sees.
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"
#include "map.h"
#include "minimap.h"
#include "player.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Table for rock removable.
**	@todo	Johns: I don't think this table or routines look correct.
**		But they work correct.
*/
global int RockTable[20];

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Check if the seen tile-type is rock.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global int MapIsSeenTileRock(int x, int y)
{
    return TheMap.Tileset->TileTypeTable[
	    TheMap.Fields[(x)+(y)*TheMap.Width].SeenTile
	] == TileTypeRock;
}

/**
**	Correct the seen rock field, depending on the surrounding.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapFixSeenRockTile(int x, int y)
{
    int tile;
    MapField *mf;

    //  Outside of map or no rock.
    if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
	return;
    }
    if (!MapIsSeenTileRock(x,y)) {
	return;
    }

    //
    //  Calculate the correct tile. Depends on the surrounding.
    //
    tile = 0;
    if ((y - 1) < 0 || MapIsSeenTileRock(x,y-1)) {
	tile |= 1 << 0;
    }
    if ((x + 1) >= TheMap.Width || MapIsSeenTileRock(x+1,y)) {
	tile |= 1 << 1;
    }
    if ((y + 1) >= TheMap.Height || MapIsSeenTileRock(x,y+1)) {
	tile |= 1 << 2;
    }
    if ((x - 1) < 0 || MapIsSeenTileRock(x-1,y)) {
	tile |= 1 << 3;
    }
    if( tile==15 ) {			// Filter more corners.
	       if ((y - 1) > 0 && (x + 1) < TheMap.Width
		&& !MapIsSeenTileRock(x+1,y-1)) {
	    tile += 1;
	} else if ((y + 1) < TheMap.Height  && (x + 1) < TheMap.Width
		&& !MapIsSeenTileRock(x+1,y+1)) {
	    tile += 2;
	} else if ((y + 1) < TheMap.Height  && (x - 1) > 0
		&& !MapIsSeenTileRock(x-1,y+1)) {
	    tile += 3;
	} else if ((y - 1) > 0 && (x - 1) > 0
		&& !MapIsSeenTileRock(x-1,y-1)) {
	    tile += 4;
	} else {
	    return;
	}
    }
    tile = RockTable[tile];

    mf = TheMap.Fields + x + y * TheMap.Width;
    if (tile == -1) {			// No valid rock remove it.
	mf->SeenTile = TheMap.Tileset->RemovedRock;
	MapFixSeenRockNeighbors(x, y);
    } else if (mf->SeenTile == tile) {
	return;
    } else {
	mf->SeenTile = tile;
    }

    // FIXME: can this only happen if seen?
#ifdef NEW_FOW
    if (mf->Visible & (1 << ThisPlayer->Player)) {
#else
    if (mf->Flags & MapFieldVisible) {
#endif
	UpdateMinimapSeenXY(x, y);
	MarkDrawPosMap(x, y);
	MustRedraw |= RedrawMinimap;
    }
}

/**
**	Correct the surrounding seen rock fields.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapFixSeenRockNeighbors(int x, int y)
{
    MapFixSeenRockTile(x + 1, y);		// side neighbors
    MapFixSeenRockTile(x - 1, y);
    MapFixSeenRockTile(x, y + 1);
    MapFixSeenRockTile(x, y - 1);
}

/**
**	Correct the real rock field, depending on the surrounding.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapFixRockTile(int x, int y)
{
    int tile;
    MapField *mf;

    //  Outside of map or no rock.
    if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
	return;
    }
    mf = TheMap.Fields + x + y * TheMap.Width;
    if (!(mf->Flags & MapFieldRocks)) {
	return;
    }
    //
    //  Calculate the correct tile. Depends on the surrounding.
    //
    tile = 0;
    if ((y - 1) < 0 || (TheMap.Fields[x + (y - 1) * TheMap.Width].
	    Flags & MapFieldRocks)) {
	tile |= 1 << 0;
    }
    if ((x + 1) >= TheMap.Width || (TheMap.Fields[x + 1 + y * TheMap.Width].
	    Flags & MapFieldRocks)) {
	tile |= 1 << 1;
    }
    if ((y + 1) >= TheMap.Height || (TheMap.Fields[x + (y + 1) * TheMap.Width].
	    Flags & MapFieldRocks)) {
	tile |= 1 << 2;
    }
    if ((x - 1) < 0 || (TheMap.Fields[x - 1 + y * TheMap.Width].
	    Flags & MapFieldRocks)) {
	tile |= 1 << 3;
    }
    tile = RockTable[tile];

    if (tile == -1) {			// No valid rock remove it.
	MapRemoveRock(x, y);
    } else if (mf->Tile != tile) {
	mf->Tile = tile;
	UpdateMinimapXY(x, y);
#ifdef NEW_FOW
	if (mf->Visible & (1 << ThisPlayer->Player)) {
#else
	if (mf->Flags & MapFieldVisible) {
#endif
	    UpdateMinimapSeenXY(x, y);
	    MapMarkSeenTile(x, y);
	    MarkDrawPosMap(x, y);
	    MustRedraw |= RedrawMinimap;
	}
    }
}

/**
**	Correct the surrounding real rock fields.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
local void MapFixRockNeighbors(int x, int y)
{
    MapFixRockTile(x + 1, y);		// side neighbors
    MapFixRockTile(x - 1, y);
    MapFixRockTile(x, y + 1);
    MapFixRockTile(x, y - 1);
}

/**
**	Remove rock from the map.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapRemoveRock(unsigned x, unsigned y)
{
    MapField *mf;

    mf = TheMap.Fields + x + y * TheMap.Width;

    mf->Tile = TheMap.Tileset->RemovedRock;
    mf->Flags &= ~(MapFieldRocks | MapFieldUnpassable);
    mf->Value = 0;

    UpdateMinimapXY(x, y);
    MapFixRockNeighbors(x, y);

#ifdef NEW_FOW
    if (mf->Visible & (1 << ThisPlayer->Player)) {
#else
    if (mf->Flags & MapFieldVisible) {
#endif
	UpdateMinimapSeenXY(x, y);
	MapMarkSeenTile(x, y);
	MarkDrawPosMap(x, y);
	MustRedraw |= RedrawMinimap;
    }
}

//@}
