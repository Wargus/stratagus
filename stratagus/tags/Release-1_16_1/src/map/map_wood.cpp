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
/**@name map_wood.c	-	The map wood handling. */
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

#include "clone.h"
#include "map.h"
#include "minimap.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global int ForestRegeneration;		/// Forest regeneration

/**
**	Table for wood removable
*/
local int WoodTable[16] = {
//  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  A,  B,  C,  D,  E,  F
   -1, 22, -1,  1, 20, 21,  3,  2, -1,  9, -1, 23,  6,  8,  5,  4
};

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

global int MapWoodChk(int x,int y) // used by FixWood
{
  if( !INMAP(x,y) ) return 1; // outside considered wood
  // return !!(MAPFIELD(x,y).Flags & MapFieldForest);
  return (TILETYPE(MAPSEENTILE(x,y)) == TileTypeWood);
}

// FIXME: this function is called to often.
global int FixWood(int x,int y) // used by MapRemoveWood2 and PreprocessMap
{
  int tile = 0;

  if ( !INMAP(x,y) ) return 0;
  if ( MapWoodChk(x,y) == 0 ) return 0;

  #define WOOD(xx,yy) (MapWoodChk(xx,yy) != 0)
  if (WOOD(x  ,y-1)) tile |= 1<<0;
  if (WOOD(x+1,y  )) tile |= 1<<1;
  if (WOOD(x  ,y+1)) tile |= 1<<2;
  if (WOOD(x-1,y  )) tile |= 1<<3;

  tile = WoodTable[tile];
  if (tile == -1)
    MapRemoveWood(x,y);
  else
    {
    // DON'T work EGCC failure tile += TheMap.Tileset->FirstWoodTile;
    tile += 0x65;
    DebugLevel3(__FUNCTION__":%x\n", TheMap.Tileset->FirstWoodTile);
    if (MAPFIELD(x,y).SeenTile == tile) return 0;
    MAPFIELD(x,y).SeenTile =  tile;
    }
  UpdateMinimapXY(x,y);
  return 1;
}

global void MapFixWood(int x,int y)
{
    // side neighbors
    FixWood( x+1, y   );
    FixWood( x-1, y   );
    FixWood( x  , y+1 );
    FixWood( x  , y-1 );
}

/**
**	Remove wood from the map.
**
**	@param x	Map X position.
**	@param y	Map Y position.
*/
global void MapRemoveWood(unsigned x,unsigned y)
{
    MapField* mf;

    mf=TheMap.Fields+x+y*TheMap.Width;

    mf->Tile=TheMap.Tileset->NoWoodTile;
    mf->Flags &= ~(MapFieldForest|MapFieldUnpassable);
    mf->Value=0;

    UpdateMinimapXY(x,y);
    MustRedraw|=RedrawMaps;

    if( mf->Flags&MapFieldVisible ) {
	MapMarkSeenTile(x,y);
    }
}

/**
**	Regenerate forest.
*/
global void RegenerateForest(void)
{
    MapField* mf;
    MapField* tmp;
    int x;
    int y;

    if( !ForestRegeneration ) {
	return;
    }

    //
    //	Increment each value of no wood.
    //	If gown up, place new wood.
    //	FIXME: a better looking result would be fine
    for( x=0; x<TheMap.Width; ++x ) {
	for( y=0; y<TheMap.Height; ++y ) {
	    mf=TheMap.Fields+x+y*TheMap.Width;
	    if( mf->Tile==TheMap.Tileset->NoWoodTile ) {
		if( mf->Value>=ForestRegeneration
			|| ++mf->Value==ForestRegeneration )  {
		    if( x && !(mf->Flags&(MapFieldWall|MapFieldUnpassable
				    |MapFieldLandUnit|MapFieldBuilding)) ) {
			tmp=mf-TheMap.Width;
			if( tmp->Tile==TheMap.Tileset->NoWoodTile 
				&& tmp->Value>=ForestRegeneration
				&& !(tmp->Flags&(MapFieldWall|MapFieldUnpassable
				    |MapFieldLandUnit|MapFieldBuilding)) ) {

			    DebugLevel0("Real place wood\n");
			    tmp->Tile=121;
			    tmp->Value=0;
			    tmp->Flags|=MapFieldForest|MapFieldUnpassable;

			    mf->Tile=123;
			    mf->Value=0;
			    mf->Flags|=MapFieldForest|MapFieldUnpassable;
			    if( mf->Flags&MapFieldVisible ) {
				MapMarkSeenTile(x,y);
			    }
			    if( tmp->Flags&MapFieldVisible ) {
				MapMarkSeenTile(x-1,y);
			    }

			}
		    }
		}
	    }
	}
    }
}

//@}
