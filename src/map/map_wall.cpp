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
  if( !INMAP(x,y) ) return 1; // outside considered wall
  // return !!(MAPFIELD(x,y).Flags & MapFieldForest);
  if (walltype == -1)
    return (TILETYPE(MAPSEENTILE(x,y)) == TileTypeHWall ||
            TILETYPE(MAPSEENTILE(x,y)) == TileTypeOWall );
  else
    return (TILETYPE(MAPSEENTILE(x,y)) == walltype);
}

global int FixWall(int x,int y) // used by MapRemoveWall and PreprocessMap
{
  int tile;
  int walltype;

  if ( !INMAP(x,y) ) return 0;
  walltype = TILETYPE(MAPSEENTILE(x,y));
  if ( walltype != TileTypeHWall && walltype != TileTypeOWall ) return 0;

  #define WALL(xx,yy) (MapWallChk(xx,yy,walltype) != 0)
  tile = 0;
  if (WALL(x  ,y-1)) tile |= 1<<0;
  if (WALL(x+1,y  )) tile |= 1<<1;
  if (WALL(x  ,y+1)) tile |= 1<<2;
  if (WALL(x-1,y  )) tile |= 1<<3;

  tile = WallTable[tile];

  if (walltype == TileTypeHWall)
     {
     if (MAPFIELD(x,y).Value < WALL_50HP)
        tile += TheMap.Tileset->HumanWall50Tile;
      else
        tile += TheMap.Tileset->HumanWall100Tile;
     }
   else
     {
     if (MAPFIELD(x,y).Value < WALL_50HP)
        tile += TheMap.Tileset->OrcWall50Tile;
      else
        tile += TheMap.Tileset->OrcWall100Tile;
     }

// FIXME: Johns, Is this correct? Could this function be called under fog of war
    if (MAPFIELD(x,y).SeenTile == tile)
	return 0;
    MAPFIELD(x,y).SeenTile =  tile;

    UpdateMinimapXY(x,y);
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
    MustRedraw|=RedrawMaps;

    if( mf->Flags&MapFieldVisible ) {
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

    MustRedraw|=RedrawMaps;

    if( mf->Flags&MapFieldVisible ) {
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
