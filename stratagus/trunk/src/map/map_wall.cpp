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
/**@name map_wall.c	-	The map wall handling. */
//
//	(c) Copyright 1999-2001 by Vladi Shabanski
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
**	Human wall table:
**		Depends on surrounding units.
**
**	@note
**		If we support a more complex tileset format, this must be
**		calculated.
*/
local int WallHumanTable[16] = {
//     0,    1,    2,    3,    4,    5,    6,    7,
      0x90,0x830,0x810,0x850,0x800,0x840,0x820,0x860,
//     8,    9,    A,    B,    C,    D,    E,    F
     0x870,0x8B0,0x890,0x8D0,0x880,0x8C0,0x8A0, 0xB0
};

/**
**	Orc wall table:
**		Depends on surrounding units.
**
**	@note
**		If we support a more complex tileset format, this must be
**		calculated.
*/
local int WallOrcTable[16] = {
//     0,    1,    2,    3,    4,    5,    6,    7,
      0xA0,0x930,0x910,0x950,0x900,0x940,0x920,0x960,
//     8,    9,    A,    B,    C,    D,    E,    F
     0x970,0x9B0,0x990,0x9D0,0x980,0x9C0,0x9A0, 0xC0
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Fix walls (connections)
----------------------------------------------------------------------------*/

/*
  Vladi:
  NOTE: this is not the original behaviour of the wall demolishing,
  instead I'm replacing tiles just as the wood fixing, so if part of
  a wall is down side neighbours are fixed just as current tile is
  empty one. It is still nice... :)

  For the connecting new walls -- all's fine.
*/

/**
**	Check if the seen tile-type is wall.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
**	@param walltype	Walltype to check. (-1 any kind)
*/
global int MapIsSeenTileWall(int x, int y,int walltype)
{
    int t;

    t = TheMap.Tileset->TileTypeTable
	    [TheMap.Fields[(x) + (y) * TheMap.Width]. SeenTile];
    if (walltype == -1) {
	return t == TileTypeHumanWall || t == TileTypeOrcWall;
    }
    return t == walltype;
}

/**
**	Correct the seen wall field, depending on the surrounding.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapFixSeenWallTile(int x, int y)
{
    int t;
    int tile;
    MapField *mf;

    //  Outside of map or no wall.
    if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
	return;
    }
    mf = TheMap.Fields + x + y * TheMap.Width;
    t = TheMap.Tileset->TileTypeTable[mf->SeenTile];
    if (t != TileTypeHumanWall && t != TileTypeOrcWall) {
	return;
    }

    //
    //  Calculate the correct tile. Depends on the surrounding.
    //
    tile = 0;
    if ((y - 1) < 0 || MapIsSeenTileWall(x, y - 1, t)) {
	tile |= 1 << 0;
    }
    if ((x + 1) >= TheMap.Width || MapIsSeenTileWall(x + 1, y, t)) {
	tile |= 1 << 1;
    }
    if ((y + 1) >= TheMap.Height || MapIsSeenTileWall(x, y + 1, t)) {
	tile |= 1 << 2;
    }
    if ((x - 1) < 0 || MapIsSeenTileWall(x - 1, y, t)) {
	tile |= 1 << 3;
    }

    if( t==TileTypeHumanWall ) {
	tile = WallHumanTable[tile];
	if( mf->Value<=UnitTypeHumanWall->_HitPoints/2 ) {
	    while( TheMap.Tileset->Table[tile] ) {	// Skip good tiles
		++tile;
	    }
	    while( !TheMap.Tileset->Table[tile] ) {	// Skip separator
		++tile;
	    }
	}
    } else {
	tile = WallOrcTable[tile];
	if( mf->Value<=UnitTypeOrcWall->_HitPoints/2 ) {
	    while( TheMap.Tileset->Table[tile] ) {	// Skip good tiles
		++tile;
	    }
	    while( !TheMap.Tileset->Table[tile] ) {	// Skip separator
		++tile;
	    }
	}
    }
    tile = TheMap.Tileset->Table[tile];

    if (mf->SeenTile != tile) {		// Already there!
	mf->SeenTile = tile;

	// FIXME: can this only happen if seen?
#ifdef NEW_FOW
	if (mf->Visible[ThisPlayer->Player]>1) {
#else
#ifdef NEW_FOW2
	if ( IsMapFieldVisible(x,y) ) {
#else
	if (mf->Flags & MapFieldVisible) {
#endif
#endif
	    UpdateMinimapSeenXY(x, y);
	    MarkDrawPosMap(x, y);
	    MustRedraw |= RedrawMinimap;
	}
    }
}

/**
**	Correct the surrounding seen wall fields.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapFixSeenWallNeighbors(int x, int y)
{
    MapFixSeenWallTile(x + 1, y);		// side neighbors
    MapFixSeenWallTile(x - 1, y);
    MapFixSeenWallTile(x, y + 1);
    MapFixSeenWallTile(x, y - 1);
}

/**
**	Correct the real wall field, depending on the surrounding.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapFixWallTile(int x, int y)
{
    int tile;
    MapField *mf;
    int t;

    //  Outside of map or no wall.
    if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
	return;
    }
    mf = TheMap.Fields + x + y * TheMap.Width;
    if (!(mf->Flags & MapFieldWall)) {
	return;
    }

    t=mf->Flags & (MapFieldHuman|MapFieldWall);
    //
    //  Calculate the correct tile. Depends on the surrounding.
    //
    tile = 0;
    if ((y - 1) < 0 || (TheMap.Fields[x + (y - 1) * TheMap.Width].
	    Flags & t) == t) {
	tile |= 1 << 0;
    }
    if ((x + 1) >= TheMap.Width || (TheMap.Fields[x + 1 + y * TheMap.Width].
	    Flags & t) == t) {
	tile |= 1 << 1;
    }
    if ((y + 1) >= TheMap.Height || (TheMap.Fields[x + (y + 1) * TheMap.Width].
	    Flags & t) == t) {
	tile |= 1 << 2;
    }
    if ((x - 1) < 0 || (TheMap.Fields[x - 1 + y * TheMap.Width].
	    Flags & t) == t) {
	tile |= 1 << 3;
    }

    if( t&MapFieldHuman ) {
	tile = WallHumanTable[tile];
	if( mf->Value<=UnitTypeHumanWall->_HitPoints/2 ) {
	    while( TheMap.Tileset->Table[tile] ) {	// Skip good tiles
		++tile;
	    }
	    while( !TheMap.Tileset->Table[tile] ) {	// Skip separator
		++tile;
	    }
	}
    } else {
	tile = WallOrcTable[tile];
	if( mf->Value<=UnitTypeOrcWall->_HitPoints/2 ) {
	    while( TheMap.Tileset->Table[tile] ) {	// Skip good tiles
		++tile;
	    }
	    while( !TheMap.Tileset->Table[tile] ) {	// Skip separator
		++tile;
	    }
	}
    }
    tile = TheMap.Tileset->Table[tile];

    if( mf->Tile != tile ) {
	mf->Tile = tile;
	UpdateMinimapXY(x, y);
#ifdef NEW_FOW
	if (mf->Visible[ThisPlayer->Player]>1) {
#else
#ifdef NEW_FOW2
	if ( IsMapFieldVisible(x,y) ) {
#else
	if (mf->Flags & MapFieldVisible) {
#endif
#endif
	    UpdateMinimapSeenXY(x, y);
	    MapMarkSeenTile(x, y);
	    MarkDrawPosMap(x, y);
	    MustRedraw |= RedrawMinimap;
	}
    }
}

/**
**	Correct the surrounding real wall fields.
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
local void MapFixWallNeighbors(int x, int y)
{
    MapFixWallTile(x + 1, y);		// side neighbors
    MapFixWallTile(x - 1, y);
    MapFixWallTile(x, y + 1);
    MapFixWallTile(x, y - 1);
}

/**
**	Remove wall from the map.
**
**	@param x	Map X position.
**	@param y	Map Y position.
*/
global void MapRemoveWall(unsigned x,unsigned y)
{
    MapField* mf;

    mf=TheMap.Fields+x+y*TheMap.Width;
    // FIXME: support more walls of different races.
    if( mf->Flags&MapFieldHuman ) {
	// FIXME: must search the correct tile
	mf->Tile=TheMap.Tileset->Table[WallHumanTable[0]+4];
    } else {
	// FIXME: must search the correct tile
	mf->Tile=TheMap.Tileset->Table[WallOrcTable[0]+4];
    }
    mf->Flags &= ~(MapFieldHuman|MapFieldWall|MapFieldUnpassable);

    UpdateMinimapXY(x,y);
    MapFixWallTile(x, y);
    MapFixWallNeighbors(x, y);

#ifdef NEW_FOW
    if (mf->Visible[ThisPlayer->Player]>1) {
#else
#ifdef NEW_FOW2
    if ( IsMapFieldVisible(x,y) ) {
#else
    if( mf->Flags&MapFieldVisible ) {
#endif
#endif
	UpdateMinimapSeenXY(x,y);
	MapMarkSeenTile(x,y);
        MarkDrawPosMap(x,y);
	MustRedraw|=RedrawMinimap;
    }
}

/**
**	Set wall onto the map.
**
**	@param x	Map X position.
**	@param y	Map Y position.
*/
global void MapSetWall(unsigned x,unsigned y,int humanwall)
{
    MapField* mf;

    mf=TheMap.Fields+x+y*TheMap.Width;

    // FIXME: support more walls of different races.
    if( humanwall ) {
	mf->Tile=TheMap.Tileset->Table[WallHumanTable[0]];
	mf->Flags|=MapFieldWall|MapFieldUnpassable|MapFieldHuman;
	mf->Value=UnitTypeHumanWall->_HitPoints;
    } else {
	mf->Tile=TheMap.Tileset->Table[WallOrcTable[0]];
	mf->Flags|=MapFieldWall|MapFieldUnpassable;
	mf->Value=UnitTypeOrcWall->_HitPoints;
    }

    UpdateMinimapXY(x,y);
    MapFixWallTile(x, y);
    MapFixWallNeighbors(x, y);

#ifdef NEW_FOW
    if (mf->Visible[ThisPlayer->Player]>1) {
#else
#ifdef NEW_FOW2
    if ( IsMapFieldVisible(x,y) ) {
#else
    if( mf->Flags&MapFieldVisible ) {
#endif
#endif
	UpdateMinimapSeenXY(x,y);
	MapMarkSeenTile(x,y);
        MarkDrawPosMap(x,y);
	MustRedraw|=RedrawMinimap;
    }
}

/**
**	Wall is hit with damage.
**
**	@param x	Map X tile-position of wall.
**	@param y	Map Y tile-position of wall.
**	@param damage	Damage done to wall.
*/
global void HitWall(unsigned x,unsigned y,unsigned damage)
{
    unsigned v;

    v=TheMap.Fields[x+y*TheMap.Width].Value;
    if( v<damage ) {
	MapRemoveWall(x,y);
    } else {
	TheMap.Fields[x+y*TheMap.Width].Value=v-damage;
	MapFixWallTile(x,y);
    }
}

//@}
