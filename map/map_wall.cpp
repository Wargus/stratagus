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

#define WALL_100HP   40 // FIXME: Vladi: should be fixed!!!
#define WALL_50HP    20 // FIXME: Vladi: should be fixed!!!

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Wall table:
**		Depends on surrounding units.
*/
local int WallTable[16] = {
//   0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  A,  B,  C,  D,  E,  F
     0,  4,  2,  7,  1,  5,  3,  8,  9, 14, 11, 16, 10, 15, 13, 17
/*
    0x90
    0x92
    0xA0
    0x94
*/
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
  empty one ( current is first NoWall tile ). It is still nice... :)

  For the connecting new walls -- all's fine.
*/

global int MapWallChk(int x,int y,int walltype) // used by FixWall, walltype==-1 for auto
{
    int t;

    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	return 1;	// Outside considered wall
    }
    t=TheMap.Tileset->TileTypeTable[
	    TheMap.Fields[(x)+(y)*TheMap.Width].SeenTile];
    if (walltype == -1) {
	return t == TileTypeHumanWall || t == TileTypeOrcWall ;
    }
    return t == walltype;
}

global int FixWall(int x, int y)	// used by MapRemoveWall and PreprocessMap
{
    int tile;
    int walltype;
    MapField *mf;

    //
    //    Outside the map
    //
    if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
	return 0;
    }
    mf = TheMap.Fields + (x) + (y) * TheMap.Width;
    walltype = TheMap.Tileset->TileTypeTable[mf->SeenTile];
    if (walltype != TileTypeHumanWall && walltype != TileTypeOrcWall) {
	return 0;
    }
#define WALL(xx,yy) (MapWallChk(xx,yy,walltype) != 0)
    tile = 0;
    if (WALL(x, y - 1)) {
	tile |= 1 << 0;
    }
    if (WALL(x + 1, y)) {
	tile |= 1 << 1;
    }
    if (WALL(x, y + 1)) {
	tile |= 1 << 2;
    }
    if (WALL(x - 1, y)) {
	tile |= 1 << 3;
    }

    tile = WallTable[tile];

    if (walltype == TileTypeHumanWall) {
	if (mf->Value < WALL_50HP)
	    tile += TheMap.Tileset->HumanWall50Tile;
	else
	    tile += TheMap.Tileset->HumanWall100Tile;
    } else {
	if (mf->Value < WALL_50HP)
	    tile += TheMap.Tileset->OrcWall50Tile;
	else
	    tile += TheMap.Tileset->OrcWall100Tile;
    }

// FIXME: Johns, Is this correct? Could this function be called under fog of war
    if (mf->SeenTile == tile) {
	return 0;
    }
    mf->SeenTile = tile;

    UpdateMinimapXY(x, y);
    return 1;
}

// this one should be called and from the HitUnit() or similar func,
// when the HP(Value) goes below 50%
global void MapFixWall(int x,int y)
{
    // side neighbors
    FixWall( x+1, y   );
    FixWall( x-1, y   );
    FixWall( x  , y+1 );
    FixWall( x  , y-1 );
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

    mf->Tile=mf->Flags&MapFieldHuman
	    ? TheMap.Tileset->HumanNoWallTile	// see vladi's FIXME
	    : TheMap.Tileset->OrcNoWallTile;	// see vladi's FIXME
    mf->Flags &= ~(MapFieldHuman|MapFieldWall|MapFieldUnpassable);

    UpdateMinimapXY(x,y);

#ifdef NEW_FOW
    if( mf->Visible&(1<<ThisPlayer->Player) ) {
#else
    if( mf->Flags&MapFieldVisible ) {
#endif
        MarkDrawPosMap(x,y);
	MustRedraw|=RedrawMinimap;
	MapMarkSeenTile(x,y);
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

    mf->Tile   = humanwall
			?TheMap.Tileset->HumanWall100Tile
			:TheMap.Tileset->OrcWall100Tile;
    mf->Flags |= humanwall?MapFieldHuman:0;
    mf->Flags |= MapFieldWall|MapFieldUnpassable;
    // FIXME: Vladi: this HP should be taken from the unit type stats!!!
    mf->Value = 40;

    UpdateMinimapXY(x,y);


#ifdef NEW_FOW
    if( mf->Visible&(1<<ThisPlayer->Player) ) {
#else
    if( mf->Flags&MapFieldVisible ) {
#endif
        MarkDrawPosMap(x,y);
	MustRedraw|=RedrawMinimap;
	MapMarkSeenTile(x,y);
    }
}

/**
**	Wall is hit with damage.
**
**	@param x	Map X position of wall.
**	@param y	Map Y position of wall.
**	@param damage	Damage done to wall.
*/
global void HitWall(unsigned x,unsigned y,unsigned damage)
{
    unsigned v;

    DebugLevel3("Missile on wall!\n");
    v=TheMap.Fields[x+y*TheMap.Width].Value;
    if( v<damage ) {
	MapRemoveWall(x,y);
    } else {
	TheMap.Fields[x+y*TheMap.Width].Value=v-damage;
	FixWall(x,y);
    }
    DebugLevel3("Missile on wall %d!\n"
	    ,TheMap.Fields[x+y*TheMap.Width].Value);
}

//@}
