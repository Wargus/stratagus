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
--	Variables
----------------------------------------------------------------------------*/

global int ForestRegeneration;		/// Forest regeneration

/**
**	Table for wood removable.
*/
global int WoodTable[16];

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Check if the tile type is wood.
**
**	Used by @see MapFixWood and @see PreprocessMap
*/
global int MapWoodChk(int x,int y)
{
    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	return 1;		// outside considered wood
    }

    return TheMap.Tileset->TileTypeTable[
	    TheMap.Fields[(x)+(y)*TheMap.Width].SeenTile
	] == TileTypeWood;
}

// FIXME: this function is called to often.
// FIXME: docu
global int FixWood(int x,int y) // used by MapRemoveWood2 and PreprocessMap
{
    int tile;
    MapField* mf;

    //	Outside map or no wood.
    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	return 0;
    }
    if ( !MapWoodChk(x,y) ) {
	return 0;
    }

    tile = 0;
#define WOOD(xx,yy) (MapWoodChk(xx,yy) != 0)
    if (WOOD(x  ,y-1)) tile |= 1<<0;
    if (WOOD(x+1,y  )) tile |= 1<<1;
    if (WOOD(x  ,y+1)) tile |= 1<<2;
    if (WOOD(x-1,y  )) tile |= 1<<3;

    tile = WoodTable[tile];
    if (tile == -1) {
	MapRemoveWood(x,y);
    } else {
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

    mf->Tile=TheMap.Tileset->RemovedTree;
    mf->Flags &= ~(MapFieldForest|MapFieldUnpassable);
    mf->Value=0;

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
	    if( mf->Tile==TheMap.Tileset->RemovedTree ) {
		if( mf->Value>=ForestRegeneration
			|| ++mf->Value==ForestRegeneration )  {
		    if( x && !(mf->Flags&(MapFieldWall|MapFieldUnpassable
				    |MapFieldLandUnit|MapFieldBuilding)) ) {
			tmp=mf-TheMap.Width;
			if( tmp->Tile==TheMap.Tileset->RemovedTree
				&& tmp->Value>=ForestRegeneration
				&& !(tmp->Flags&(MapFieldWall|MapFieldUnpassable
				    |MapFieldLandUnit|MapFieldBuilding)) ) {

			    DebugLevel0("Real place wood\n");
			    tmp->Tile=TheMap.Tileset->TopOneTree;
			    tmp->Value=0;
			    tmp->Flags|=MapFieldForest|MapFieldUnpassable;

			    mf->Tile=TheMap.Tileset->BotOneTree;
			    mf->Value=0;
			    mf->Flags|=MapFieldForest|MapFieldUnpassable;
#ifdef NEW_FOW
			    if( mf->Visible&(1<<ThisPlayer->Player) ) {
				MapMarkSeenTile(x,y);
			    }
			    if( tmp->Visible&(1<<ThisPlayer->Player) ) {
				MapMarkSeenTile(x-1,y);
			    }
#else
			    if( mf->Flags&MapFieldVisible ) {
				MapMarkSeenTile(x,y);
			    }
			    if( tmp->Flags&MapFieldVisible ) {
				MapMarkSeenTile(x-1,y);
			    }
#endif

			}
		    }
		}
	    }
	}
    }
}

//@}
