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
//	(c) Copyright 1998-2002 by Lutz Sammer and Vladi Shabanski
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
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
global void MapMarkSeenTile(int x, int y)
{
    int tile;
    int seentile;
    MapField *mf;

    mf = TheMap.Fields + x + y * TheMap.Width;
    //
    // Nothing changed? Seeing already the correct tile.
    //
    if ((tile = mf->Tile) == (seentile = mf->SeenTile)) {
	return;
    }
    mf->SeenTile = tile;

    // FIXME: this is needed, because tileset is loaded after this function
    //          is needed LoadPud, PlaceUnit, ... MapMarkSeenTile
    if (!TheMap.Tileset) {
	return;
    }

    //
    //  Handle wood changes. FIXME: check if for growing wood correct?
    //
    if (seentile != TheMap.Tileset->RemovedTree
	    && tile == TheMap.Tileset->RemovedTree) {
	MapFixSeenWoodNeighbors(x, y);
    } else if (seentile == TheMap.Tileset->RemovedTree
	    && tile != TheMap.Tileset->RemovedTree) {
	MapFixSeenWoodTile(x, y);
    } else if (ForestOnMap(x, y)) {
	MapFixSeenWoodTile(x, y);
	MapFixSeenWoodNeighbors(x, y);

	//
	//  Handle rock changes.
	//
    } else if (seentile != TheMap.Tileset->RemovedRock
	    && tile == TheMap.Tileset->RemovedRock) {
	MapFixSeenRockNeighbors(x, y);
    } else if (seentile == TheMap.Tileset->RemovedRock
	    && tile != TheMap.Tileset->RemovedRock) {
	MapFixSeenRockTile(x, y);
    } else if (RockOnMap(x, y)) {
	MapFixSeenRockTile(x, y);
	MapFixSeenRockNeighbors(x, y);

	//
	//  Handle Walls changes.
	//
    } else if (TheMap.Tileset->TileTypeTable[tile] == TileTypeHumanWall
	    || TheMap.Tileset->TileTypeTable[tile] == TileTypeOrcWall
	    || TheMap.Tileset->TileTypeTable[seentile] == TileTypeHumanWall
	    || TheMap.Tileset->TileTypeTable[seentile] == TileTypeOrcWall) {
	MapFixSeenWallTile(x, y);
	MapFixSeenWallNeighbors(x, y);
    }
}

/**
**	Reveal the entire map.
*/
global void RevealMap(void)
{
    int ix;
    int iy;

    for ( ix = 0; ix < TheMap.Width; ix++ ) {
	for ( iy = 0; iy < TheMap.Height; iy++ ) {
#ifdef NEW_FOW
	    if( TheMap.NoFogOfWar ) {
		TheMap.Fields[ix+iy*TheMap.Width].Visible[ThisPlayer->Player]=2;
	    } else {
		TheMap.Fields[ix+iy*TheMap.Width].Visible[ThisPlayer->Player]=1;
	    }
#else
	    TheMap.Fields[ix+iy*TheMap.Width].Flags |= MapFieldExplored;

	    if( TheMap.NoFogOfWar ) {
		TheMap.Visible[0][((iy)*TheMap.Width+(ix))/32] |= 
			(1<<(((iy)*TheMap.Width+(ix))%32));
	    }
#endif
	    MapMarkSeenTile(ix,iy);
	}
    }
}

/**
**	Change viewpoint of map viewport v to x,y.
**
**	@param v	The viewport.
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
global void MapViewportSetViewpoint(int v, int x, int y)
{
    int map_width;
    int map_height;

    if (x == TheUI.VP[v].MapX && y == TheUI.VP[v].MapY) {
	return;
    }

    map_width = TheUI.VP[v].MapWidth;
    if (x < 0) {
	TheUI.VP[v].MapX = 0;
    } else if (x > TheMap.Width - map_width) {
	TheUI.VP[v].MapX = TheMap.Width - map_width;
    } else {
	TheUI.VP[v].MapX = x;
    }

    map_height = TheUI.VP[v].MapHeight;
    if (y < 0) {
	TheUI.VP[v].MapY = 0;
    } else if (y > TheMap.Height - map_height) {
	TheUI.VP[v].MapY = TheMap.Height - map_height;
    } else {
	TheUI.VP[v].MapY = y;
    }
    MarkDrawEntireMap();
    MustRedraw |= RedrawMinimap | RedrawMinimapCursor;
}

/**
**	Center map viewport v on map tile (x,y).
**
**	@param v	The viewport.
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
global void MapCenterViewport(int v, int x, int y)
{
    MapViewportSetViewpoint(v,
	    x - (TheUI.VP[v].MapWidth / 2), y - (TheUI.VP[v].MapHeight / 2));
}

/*----------------------------------------------------------------------------
--	Map queries
----------------------------------------------------------------------------*/

/**
**	Tile is empty, no rocks, walls, forest, building?
**
**	@param tx	X map tile position.
**	@param ty	Y map tile position.
**
**	@return		True if empty, false otherwise.
*/
global int IsMapFieldEmpty(int tx,int ty)
{
    return !(TheMap.Fields[tx+ty*TheMap.Width].Flags
	    &(MapFieldUnpassable|MapFieldWall|MapFieldRocks|MapFieldForest
	    |MapFieldBuilding));
}

/**
**	Water on map tile.
**
**	@param tx	X map tile position.
**	@param ty	Y map tile position.
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
**	@param tx	X map tile position.
**	@param ty	Y map tile position.
**	@return		True if coast, false otherwise.
*/
global int CoastOnMap(int tx,int ty)
{
    return TheMap.Fields[tx+ty*TheMap.Width].Flags&MapFieldCoastAllowed;
}

/**
**	Wall on map tile.
**
**	@param tx	X map tile position.
**	@param ty	Y map tile position.
**	@return		True if wall, false otherwise.
*/
global int WallOnMap(int tx,int ty)
{
    return TheMap.Fields[tx+ty*TheMap.Width].Flags&MapFieldWall;
}

/**
**	Human wall on map tile.
**
**	@param tx	X map tile position.
**	@param ty	Y map tile position.
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
**	@param tx	X map tile position.
**	@param ty	Y map tile position.
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
**	@param tx	X map tile position.
**	@param ty	Y map tile position.
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
**	@param tx	X map tile position.
**	@param ty	Y map tile position.
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
**	@param tx	X map tile position.
**	@param ty	Y map tile position.
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

#ifdef HIERARCHIC_PATHFINDER

/**
**	FIXME: write the description of this function.
*/
global inline unsigned short MapFieldGetRegId(int tx, int ty)
{
    return TheMap.Fields[ty * TheMap.Width + tx].RegId;
}

/**
**	FIXME: write the description of this function.
*/
global inline void MapFieldSetRegId(int tx, int ty, unsigned short regid)
{
    TheMap.Fields[ty * TheMap.Width + tx].RegId = regid;
}

/**
**	FIXME: write the description of this function.
** 	@todo FIXME: convert to a macro
*/
global inline int MapFieldPassable(int tx, int ty, int mask)
{
    return !(TheMap.Fields[ty * TheMap.Width + tx].Flags & mask);
}

#endif // HIERARCHIC_PATHFINDER

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
**	Can an unit of unit-type move to this point.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@param type	unit-type to be checked.
**
**	@return		True if could be entered, false otherwise.
*/
global int UnitTypeCanMoveTo(int x,int y,const UnitType* type)
{
    return CanMoveToMask(x,y,TypeMovementMask(type));
}

/**
**	Can an unit move to this point.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@param unit	unit to be checked.
**
**	@return		True if could be entered, false otherwise.
*/
global int UnitCanMoveTo(int x,int y,const Unit* unit)
{
    return CanMoveToMask(x,y,TypeMovementMask(unit->Type));
}

#if 0

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

#endif

/**
**	Fixes initially the wood and seen tiles.
*/
global void PreprocessMap(void)
{
    int ix;
    int iy;
    MapField* mf;

    for (ix = 0; ix < TheMap.Width; ix++) {
	for (iy = 0; iy < TheMap.Height; iy++) {
	    mf = TheMap.Fields + ix + iy * TheMap.Width;
	    mf->SeenTile = mf->Tile;
	}
    }

    // it is required for fixing the wood that all tiles are marked as seen!
    for (ix = 0; ix < TheMap.Width; ix++) {
	for (iy = 0; iy < TheMap.Height; iy++) {
	    MapFixWoodTile(ix, iy);
	    MapFixSeenWoodTile(ix, iy);
	    MapFixRockTile(ix, iy);
	    MapFixSeenRockTile(ix, iy);
	    MapFixWallTile(ix, iy);
	    MapFixSeenWallTile(ix, iy);
	}
    }
}

/**
**	Convert viewport x coordinate to map tile x coordinate.
**
**	@param v	The viewport.
**	@param x	X coordinate into this viewport (in pixels, relative
**			to origin of FreeCraft's window - not the viewport
**			itself!).
**
**	@return		X map tile coordinate.
*/
global int Viewport2MapX(int v, int x)
{
    int r;

    r = ((x) - TheUI.VP[v].X) / TileSizeX + TheUI.VP[v].MapX;
    return r < TheMap.Width ? r : TheMap.Width - 1;
}

/**
**	Convert viewport y coordinate to map tile y coordinate.
**
**	@param v	The viewport.
**	@param y	Y coordinate into this viewport (in pixels, relative
**			to origin of FreeCraft's window - not the viewport
**			itself!).
**
**	@return		Y map tile coordinate.
*/
global int Viewport2MapY(int v, int y)
{
    int r;

    r = ((y) - TheUI.VP[v].Y) / TileSizeY + TheUI.VP[v].MapY;
    return r < TheMap.Height ? r : TheMap.Height - 1;
}

/**
**	Convert a map tile X coordinate into a viewport x pixel coordinate.
**
**	@param v	The viewport.
**	@param x	The map tile's X coordinate.
**
**	@return		X screen coordinate in pixels (relative
**                      to origin of FreeCraft's window).
*/
global int Map2ViewportX(int v, int x)
{
    return TheUI.VP[v].X + ((x) - TheUI.VP[v].MapX) * TileSizeX;
}

/**
**	Convert a map tile Y coordinate into a viewport y pixel coordinate.
**
**	@param v	The viewport.
**	@param y	The map tile's Y coordinate.
**
**	@return		Y screen coordinate in pixels (relative
**                      to origin of FreeCraft's window).
*/
global int Map2ViewportY(int v, int y)
{
    return TheUI.VP[v].Y + ((y) - TheUI.VP[v].MapY) * TileSizeY;
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
	if (info->MapTerrainName) {
	    free(info->MapTerrainName);
	}
	free(info);
    }
}

/**
**	Cleanup the map module.
*/
global void CleanMap(void)
{
    free(TheMap.Fields);
    free(TheMap.TerrainName);
    free(TheMap.Visible[0]);

    // Tileset freeed by Tileset?

    FreeMapInfo(TheMap.Info);
    memset(&TheMap, 0, sizeof(TheMap));
    FlagRevealMap = 0;

    DestroyMinimap();

    CleanMapFogOfWar();

    // FIXME: don't need to call both of these
    CleanPud();
    CleanScm();
}

//@}
