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
/**@name map.c		-	The map. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer and Vladi Shabanski
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
#include "map.h"
#include "minimap.h"
#include "player.h"
#include "unit.h"
#include "pathfinder.h"
#include "pud.h"
#include "ui.h"

#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global WorldMap TheMap;			/// The current map

global unsigned MapX;			/// Map tile X start on display
global unsigned MapY;			/// Map tile Y start on display
global unsigned MapWidth;		/// Map width in tiles
global unsigned MapHeight;		/// Map height in tiles

global int FlagRevealMap;		/// Flag must reveal the map

/*----------------------------------------------------------------------------
--	Visibile and explored handling
----------------------------------------------------------------------------*/

/**
**      Marks seen tile -- used mainly for the Fog Of War
**
**	@param x	Map X tile-position.
**	@param y	Map Y tile-position.
*/
global void MapMarkSeenTile( int x, int y )
{
    int tile;
    int seentile;
    MapField* mf;

    mf=TheMap.Fields+x+y*TheMap.Width;
    //
    // Nothing changed? Seeing already the correct tile.
    //
    if ((tile  = mf->Tile) == (seentile = mf->SeenTile)) {
	return;
    }
    mf->SeenTile=tile;

    // FIXME: this is needed, because tileset is loaded after this function
    //		is needed LoadPud, PlaceUnit, ... MapMarkSeenTile
    if( !TheMap.Tileset ) {
	return;
    }

    //
    //	Handle wood changes. FIXME: check if for growing wood correct?
    //
    if ( seentile != TheMap.Tileset->RemovedTree
	    && tile == TheMap.Tileset->RemovedTree ) {
	MapFixSeenWoodNeighbors( x, y );
    } else if ( seentile == TheMap.Tileset->RemovedTree
	    && tile != TheMap.Tileset->RemovedTree ) {
	MapFixSeenWoodTile( x, y );
    } else if ( ForestOnMap( x, y ) ) {
	MapFixSeenWoodTile( x, y );
	MapFixSeenWoodNeighbors( x, y );

    //
    //	Handle rock changes.
    //
    } else if ( seentile != TheMap.Tileset->RemovedRock
	    && tile == TheMap.Tileset->RemovedRock ) {
	MapFixSeenRockNeighbors( x, y );
    } else if ( seentile == TheMap.Tileset->RemovedRock
	    && tile != TheMap.Tileset->RemovedRock ) {
	MapFixSeenRockTile( x, y );
    } else if ( RockOnMap( x, y ) ) {
	MapFixSeenRockTile( x, y );
	MapFixSeenRockNeighbors( x, y );

    //
    //	Handle Walls changes.
    //
    } else if ( TheMap.Tileset->TileTypeTable[tile] == TileTypeHumanWall
	    || TheMap.Tileset->TileTypeTable[tile] == TileTypeOrcWall
	    || TheMap.Tileset->TileTypeTable[seentile] == TileTypeHumanWall
	    || TheMap.Tileset->TileTypeTable[seentile] == TileTypeOrcWall ) {
	MapFixSeenWallTile( x, y );
	MapFixSeenWallNeighbors( x, y );
    }
}

/**
**	Reveal the entire map.
*/
global void RevealMap(void)
{
    int ix, iy;

    for ( ix = 0; ix < TheMap.Width; ix++ ) {
	for ( iy = 0; iy < TheMap.Height; iy++ ) {
#ifdef NEW_FOW
	    int m;

	    m=(1<<ThisPlayer->Player);
	    TheMap.Fields[ix+iy*TheMap.Width].Explored|=m;
	    if( TheMap.NoFogOfWar ) {
		TheMap.Fields[ix+iy*TheMap.Width].Visible|=m;
	    }
	    // FIXME: Set Mask.
#else
	    TheMap.Fields[ix+iy*TheMap.Width].Flags
		    |= MapFieldExplored
			| (TheMap.NoFogOfWar ? MapFieldVisible : 0);
#endif
	    MapMarkSeenTile(ix,iy);
	}
    }
}

/**
**	Change viewpoint of map to x,y
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
global void MapSetViewpoint(int x,int y)
{
    if (x==MapX && y==MapY) {
	return;
    }

    if( x<0 ) {
	MapX=0;
    } else if( x>TheMap.Width-MapWidth ) {
	MapX=TheMap.Width-MapWidth;
    } else {
	MapX=x;
    }
    if( y<0 ) {
	MapY=0;
    } else if( y>TheMap.Height-MapHeight ) {
	MapY=TheMap.Height-MapHeight;
    } else {
	MapY=y;
    }
    MarkDrawEntireMap();
    MustRedraw|=RedrawMinimap|RedrawMinimapCursor;
}

/**
**	Center map viewpoint on x,y.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
global void MapCenter(int x,int y)
{
    MapSetViewpoint(x-(MapWidth/2),y-(MapHeight/2));
}

/*----------------------------------------------------------------------------
--	Map queries
----------------------------------------------------------------------------*/

/**
**	Water on map tile.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@return		True if water, false otherwise.
*/
global int WaterOnMap(int tx,int ty)
{
    return TheMap.Fields[tx+ty*TheMap.Width].Flags&MapFieldWaterAllowed;
}

/**
**	Coast on map tile.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@return		True if coast, false otherwise.
*/
global int CoastOnMap(int tx,int ty)
{
    return TheMap.Fields[tx+ty*TheMap.Width].Flags&MapFieldCoastAllowed;
}

/**
**	Wall on map tile.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@return		True if wall, false otherwise.
*/
global int WallOnMap(int tx,int ty)
{
    return TheMap.Fields[tx+ty*TheMap.Width].Flags&MapFieldWall;
}

/**
**	Human wall on map tile.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@return		True if human wall, false otherwise.
*/
global int HumanWallOnMap(int tx,int ty)
{
    return (TheMap.Fields[tx+ty*TheMap.Width].Flags
	    &(MapFieldWall|MapFieldHuman))==(MapFieldWall|MapFieldHuman);
}

/**
**	Orc wall on map tile.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@return		True if orcish wall, false otherwise.
*/
global int OrcWallOnMap(int tx,int ty)
{
    return (TheMap.Fields[tx+ty*TheMap.Width].Flags
	    &(MapFieldWall|MapFieldHuman))==MapFieldWall;
}

/**
**	Forest on map tile. Checking version.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@return		True if forest, false otherwise.
*/
global int CheckedForestOnMap(int tx,int ty)
{
    if( tx<0 || ty<0 || tx>=TheMap.Width || ty>=TheMap.Height ) {
	return 0;
    }
    return TheMap.Fields[tx+ty*TheMap.Width].Flags&MapFieldForest;
}

/**
**	Forest on map tile.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@return		True if forest, false otherwise.
*/
global int ForestOnMap(int tx,int ty)
{
    IfDebug(
	if( tx<0 || ty<0 || tx>=TheMap.Width || ty>=TheMap.Height ) {
	    // FIXME: must cleanup calling function !
	    fprintf(stderr,"Used x %d, y %d\n",tx,ty);
	    abort();
	    return 0;
	}
    );

    return TheMap.Fields[tx+ty*TheMap.Width].Flags&MapFieldForest;
}

/**
**	Rock on map tile.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@return		True if rock, false otherwise.
*/
global int RockOnMap(int tx,int ty)
{
    IfDebug(
	if( tx<0 || ty<0 || tx>=TheMap.Width || ty>=TheMap.Height ) {
	    // FIXME: must cleanup calling function !
	    fprintf(stderr,"Used x %d, y %d\n",tx,ty);
	    abort();
	    return 0;
	}
    );

    return TheMap.Fields[tx+ty*TheMap.Width].Flags&MapFieldRocks;
}

/**
**	Can move to this point, applying mask.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@param mask	Mask for movement to apply.
**
**	@return		True if could be entered, false otherwise.
*/
global int CheckedCanMoveToMask(int x,int y,int mask)
{
    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	return 0;
    }

    return !(TheMap.Fields[x+y*TheMap.Width].Flags&mask);
}

#ifndef CanMoveToMask
/**
**	Can move to this point, applying mask.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@param mask	Mask for movement to apply.
**
**	@return		True if could be entered, false otherwise.
*/
global int CanMoveToMask(int x,int y,int mask)
{
    IfDebug(
	if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	    // FIXME: must cleanup calling function !
	    fprintf(stderr,"Used x %d, y %d, mask %x\n",x,y,mask);
	    abort();
	    return 0;
	}
    );

    return !(TheMap.Fields[x+y*TheMap.Width].Flags&mask);
}
#endif

/**
**	Return the units field flags.
**	This flags are used to mark the field for this unit.
**
**	@param unit	Pointer to unit.
**
**	@return		Field flags to be set.
*/
global unsigned UnitFieldFlags(const Unit* unit)
{
    // FIXME: Should be moved into unittype structure, and allow more types.
    switch( unit->Type->UnitType ) {
	case UnitTypeLand:		// on land
	    return MapFieldLandUnit;
	case UnitTypeFly:		// in air
	    return MapFieldAirUnit;
	case UnitTypeNaval:		// on water
	    return MapFieldSeaUnit;
	default:
	    DebugLevel1Fn("Were moves this unit?\n");
	    return 0;
    }
}

/**
**	Return the unit type movement mask.
**		TODO: Should add this to unit-type structure.
**
**	@param type	Unit type pointer.
**
**	@return		Movement mask of unit type.
*/
global int TypeMovementMask(const UnitType* type)
{
    // FIXME: Should be moved into unittype structure, and allow more types.
    switch( type->UnitType ) {
	case UnitTypeLand:		// on land
	    return MapFieldLandUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldWall
		| MapFieldRocks
		| MapFieldForest	// wall,rock,forest not 100% clear?
		| MapFieldCoastAllowed
		| MapFieldWaterAllowed	// can't move on this
		| MapFieldUnpassable;
	case UnitTypeFly:		// in air
	    return MapFieldAirUnit;	// already occuppied
	case UnitTypeNaval:		// on water
	    if( type->Transporter ) {
		return MapFieldLandUnit
		    | MapFieldSeaUnit
		    | MapFieldBuilding	// already occuppied
		    | MapFieldLandAllowed;	// can't move on this
		    //| MapFieldUnpassable;	// FIXME: bug?
	    }
	    return MapFieldSeaUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldCoastAllowed
		| MapFieldLandAllowed	// can't move on this
		| MapFieldUnpassable;
	default:
	    DebugLevel1Fn("Were moves this unit?\n");
	    return 0;
    }
}

/**
**	Return units movement mask.
**
**	@param unit	Unit pointer.
**
**	@return		Movement mask of unit.
*/
global int UnitMovementMask(const Unit* unit)
{
    return TypeMovementMask(unit->Type);
}

/**
**	Fixes initially the wood and seen tiles.
*/
global void PreprocessMap(void)
{
    unsigned ix, iy;
    MapField* mf;

    for ( ix = 0; ix < TheMap.Width; ix++ ) {
	for ( iy = 0; iy < TheMap.Height; iy++ ) {
	    mf=TheMap.Fields+ix+iy*TheMap.Width;
	    mf->SeenTile=mf->Tile;
	}
    }

    // it is required for fixing the wood that all tiles are marked as seen!
    for ( ix = 0; ix < TheMap.Width; ix++ ) {
	for ( iy = 0; iy < TheMap.Height; iy++ ) {
	    MapFixWoodTile( ix, iy );
	    MapFixSeenWoodTile( ix, iy );
	    MapFixRockTile( ix, iy );
	    MapFixSeenRockTile( ix, iy );
	    MapFixWallTile( ix, iy );
	    MapFixSeenWallTile( ix, iy );
	}
    }
}

/**
**	Convert a screen coordinate to map tile.
**
**	@param x	X screen coordinate.
**
**	@return		X tile number.
*/
global int Screen2MapX(int x)
{
    return (((x)-TheUI.MapX)/TileSizeX+MapX);
}

/**
**	Convert a screen coordinate to map tile.
**
**	@param y	Y screen coordinate.
**
**	@return		Y tile number.
*/
global int Screen2MapY(int y)
{
    return (((y)-TheUI.MapY)/TileSizeY+MapY);
}

/**
**	Convert a map tile into screen coordinate.
**
**	@param x	X tile number.
**
**	@return		X screen coordinate.
*/
global int Map2ScreenX(int x)
{
    return (TheUI.MapX+((x)-MapX)*TileSizeX);
}

/**
**	Convert a map tile into screen coordinate.
**
**	@param y	Y tile number.
**
**	@return		Y screen coordinate.
*/
global int Map2ScreenY(int y)
{
    return (TheUI.MapY+((y)-MapY)*TileSizeY);
}

/**
**	Release info about a map.
**
**	@param info	MapInfo pointer.
*/
global void FreeMapInfo(MapInfo* info)
{
    if (info) {
	if (info->Description) {
	    free(info->Description);
	}
	free(info);
    }
}

//@}
