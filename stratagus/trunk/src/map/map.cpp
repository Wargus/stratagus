//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name map.c		-	The map. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Vladi Shabanski
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "map.h"
#include "minimap.h"
#include "player.h"
#include "unit.h"
#include "pathfinder.h"
#include "pud.h"
#include "ui.h"
#include "editor.h"

#include "script.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

global WorldMap TheMap;						/// The current map
global int FlagRevealMap;				/// Flag must reveal the map
global int ReplayRevealMap;				/// Reveal Map is replay

/*----------------------------------------------------------------------------
--		Visible and explored handling
----------------------------------------------------------------------------*/

/**
**	  Marks seen tile -- used mainly for the Fog Of War
**
**		@param x		Map X tile-position.
**		@param y		Map Y tile-position.
*/
global void MapMarkSeenTile(int x, int y)
{
	int tile;
	int seentile;
	MapField* mf;

	mf = TheMap.Fields + x + y * TheMap.Width;
	//
	// Nothing changed? Seeing already the correct tile.
	//
	if ((tile = mf->Tile) == (seentile = mf->SeenTile)) {
		// FIXME: Check bounds
		MarkDrawAreaMap(x - 1, y - 1, x + 1, y + 1);
		return;
	}
	mf->SeenTile = tile;

	// FIXME: this is needed, because tileset is loaded after this function
	//		  is needed LoadPud, PlaceUnit, ... MapMarkSeenTile
	if (!TheMap.Tileset) {
		return;
	}

	//
	//  Handle wood changes. FIXME: check if for growing wood correct?
	//
	if (seentile != TheMap.Tileset->RemovedTree &&
			tile == TheMap.Tileset->RemovedTree) {
		MapFixSeenWoodNeighbors(x, y);
	} else if (seentile == TheMap.Tileset->RemovedTree &&
			tile != TheMap.Tileset->RemovedTree) {
		MapFixSeenWoodTile(x, y);
	} else if (ForestOnMap(x, y)) {
		MapFixSeenWoodTile(x, y);
		MapFixSeenWoodNeighbors(x, y);

		//
		//  Handle rock changes.
		//
	} else if (seentile != TheMap.Tileset->RemovedRock &&
			tile == TheMap.Tileset->RemovedRock) {
		MapFixSeenRockNeighbors(x, y);
	} else if (seentile == TheMap.Tileset->RemovedRock &&
			tile != TheMap.Tileset->RemovedRock) {
		MapFixSeenRockTile(x, y);
	} else if (RockOnMap(x, y)) {
		MapFixSeenRockTile(x, y);
		MapFixSeenRockNeighbors(x, y);

		//
		//  Handle Walls changes.
		//
	} else if (TheMap.Tileset->TileTypeTable[tile] == TileTypeHumanWall ||
			TheMap.Tileset->TileTypeTable[tile] == TileTypeOrcWall ||
			TheMap.Tileset->TileTypeTable[seentile] == TileTypeHumanWall ||
			TheMap.Tileset->TileTypeTable[seentile] == TileTypeOrcWall) {
		MapFixSeenWallTile(x, y);
		MapFixSeenWallNeighbors(x, y);
	}
}

/**
**		Reveal the entire map.
*/
global void RevealMap(void)
{
	int x;
	int y;
	int p;

	//
	//	Mark every explored tile as visible. 1 turns into 2.
	//
	for (x = 0; x < TheMap.Width; ++x) {
		for (y = 0; y < TheMap.Height; ++y) {
			int i;
			for (i = 0; i < PlayerMax; ++i) {
				if (!TheMap.Fields[x+y*TheMap.Width].Visible[i]) {
					TheMap.Fields[x+y*TheMap.Width].Visible[i] = 1;
				}
			}
			MapMarkSeenTile(x, y);
		}
	}
	//
	//	Global seen recount. Simple and effective.
	//
	for (x = 0; x < NumUnits; ++x) {
		//
		//	Reveal neutral buildings. Gold mines:)
		//
		if (Units[x]->Player->Type == PlayerNeutral) {
			for (p = 0; p < PlayerMax; ++p) {
				if (Players[p].Type != PlayerNobody &&
						(!(Units[x]->Seen.ByPlayer & (1 << p)))) {
					if (1) {
						UnitGoesOutOfFog(Units[x], Players + p);
						UnitGoesUnderFog(Units[x], Players + p);
					}
				}
			}
		}
		UnitCountSeen(Units[x]);
	}
}

/**
**		Change viewpoint of map viewport v to x,y.
**
**		@param vp		Viewport pointer.
**		@param x		X map tile position.
**		@param y		Y map tile position.
*/
global void ViewportSetViewpoint(Viewport* vp, int x, int y)
{
	int map_width;
	int map_height;

	if (x == vp->MapX && y == vp->MapY) {
		return;								// Already using this view.
	}

	map_width = vp->MapWidth;
	if (x < 0) {
		vp->MapX = 0;
	} else if (x > TheMap.Width - map_width) {
		vp->MapX = TheMap.Width - map_width;
	} else {
		vp->MapX = x;
	}

	map_height = vp->MapHeight;
	if (y < 0) {
		vp->MapY = 0;
	} else if (y > TheMap.Height - map_height) {
		vp->MapY = TheMap.Height - map_height;
	} else {
		vp->MapY = y;
	}

	MarkDrawEntireMap();
	MustRedraw |= RedrawMinimap | RedrawMinimapCursor;
}

/**
**		Center map viewport v on map tile (x,y).
**
**		@param vp		Viewport pointer.
**		@param x		X map tile position.
**		@param y		Y map tile position.
*/
global void ViewportCenterViewpoint(Viewport* vp, int x, int y)
{
	ViewportSetViewpoint(vp, x - (vp->MapWidth / 2), y - (vp->MapHeight / 2));
}

/*----------------------------------------------------------------------------
--		Map queries
----------------------------------------------------------------------------*/

/**
**		Tile is empty, no rocks, walls, forest, building?
**
**		@param tx		X map tile position.
**		@param ty		Y map tile position.
**
**		@return				True if empty, false otherwise.
*/
global int IsMapFieldEmpty(int tx, int ty)
{
	return !(TheMap.Fields[tx + ty * TheMap.Width].Flags &
		(MapFieldUnpassable | MapFieldWall | MapFieldRocks | MapFieldForest |
			MapFieldBuilding));
}

/**
**		Water on map tile.
**
**		@param tx		X map tile position.
**		@param ty		Y map tile position.
**
**		@return				True if water, false otherwise.
*/
global int WaterOnMap(int tx, int ty)
{
	return TheMap.Fields[tx + ty * TheMap.Width].Flags & MapFieldWaterAllowed;
}

/**
**		Coast on map tile.
**
**		@param tx		X map tile position.
**		@param ty		Y map tile position.
**		@return				True if coast, false otherwise.
*/
global int CoastOnMap(int tx, int ty)
{
	return TheMap.Fields[tx + ty * TheMap.Width].Flags & MapFieldCoastAllowed;
}

/**
**		Wall on map tile.
**
**		@param tx		X map tile position.
**		@param ty		Y map tile position.
**		@return				True if wall, false otherwise.
*/
global int WallOnMap(int tx, int ty)
{
	return TheMap.Fields[tx + ty * TheMap.Width].Flags & MapFieldWall;
}

/**
**		Human wall on map tile.
**
**		@param tx		X map tile position.
**		@param ty		Y map tile position.
**		@return				True if human wall, false otherwise.
*/
global int HumanWallOnMap(int tx, int ty)
{
	return (TheMap.Fields[tx + ty * TheMap.Width].Flags &
		(MapFieldWall | MapFieldHuman)) == (MapFieldWall | MapFieldHuman);
}

/**
**		Orc wall on map tile.
**
**		@param tx		X map tile position.
**		@param ty		Y map tile position.
**		@return				True if orcish wall, false otherwise.
*/
global int OrcWallOnMap(int tx, int ty)
{
	return (TheMap.Fields[tx + ty * TheMap.Width].Flags &
		(MapFieldWall | MapFieldHuman)) == MapFieldWall;
}

/**
**		Forest on map tile. Checking version.
**
**		@param tx		X map tile position.
**		@param ty		Y map tile position.
**
**		@return				True if forest, false otherwise.
*/
global int CheckedForestOnMap(int tx, int ty)
{
	if (tx < 0 || ty < 0 || tx >= TheMap.Width || ty >= TheMap.Height) {
		return 0;
	}
	return TheMap.Fields[tx + ty * TheMap.Width].Flags & MapFieldForest;
}

/**
**		Forest on map tile.
**
**		@param tx		X map tile position.
**		@param ty		Y map tile position.
**
**		@return				True if forest, false otherwise.
*/
global int ForestOnMap(int tx, int ty)
{
#ifdef DEBUG
	if (tx < 0 || ty < 0 || tx >= TheMap.Width || ty >= TheMap.Height) {
		// FIXME: must cleanup calling function !
		fprintf(stderr, "Used x %d, y %d\n", tx, ty);
		abort();
		return 0;
	}
#endif

	return TheMap.Fields[tx + ty * TheMap.Width].Flags & MapFieldForest;
}

/**
**		Rock on map tile.
**
**		@param tx		X map tile position.
**		@param ty		Y map tile position.
**
**		@return				True if rock, false otherwise.
*/
global int RockOnMap(int tx, int ty)
{
#ifdef DEBUG
	if (tx < 0 || ty < 0 || tx >= TheMap.Width || ty >= TheMap.Height) {
		// FIXME: must cleanup calling function !
		fprintf(stderr, "Used x %d, y %d\n", tx, ty);
		abort();
		return 0;
	}
#endif

	return TheMap.Fields[tx + ty * TheMap.Width].Flags & MapFieldRocks;
}

/**
**		Can move to this point, applying mask.
**
**		@param x		X map tile position.
**		@param y		Y map tile position.
**		@param mask		Mask for movement to apply.
**
**		@return				True if could be entered, false otherwise.
*/
global int CheckedCanMoveToMask(int x, int y, int mask)
{
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		return 0;
	}

	return !(TheMap.Fields[x + y * TheMap.Width].Flags & mask);
}

#ifndef CanMoveToMask
/**
**		Can move to this point, applying mask.
**
**		@param x		X map tile position.
**		@param y		Y map tile position.
**		@param mask		Mask for movement to apply.
**
**		@return				True if could be entered, false otherwise.
*/
global int CanMoveToMask(int x, int y, int mask)
{
#ifdef DEBUG
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		// FIXME: must cleanup calling function !
		fprintf(stderr, "Used x %d, y %d, mask %x\n", x, y, mask);
		abort();
		return 0;
	}
#endif

	return !(TheMap.Fields[x + y * TheMap.Width].Flags & mask);
}
#endif

/**
**		Can an unit of unit-type move to this point.
**
**		@param x		X map tile position.
**		@param y		Y map tile position.
**		@param type		unit-type to be checked.
**
**		@return				True if could be entered, false otherwise.
*/
global int UnitTypeCanMoveTo(int x, int y, const UnitType* type)
{
	return CanMoveToMask(x, y, TypeMovementMask(type));
}

/**
**		Can an unit move to this point.
**
**		@param x		X map tile position.
**		@param y		Y map tile position.
**		@param unit		unit to be checked.
**
**		@return				True if could be entered, false otherwise.
*/
global int UnitCanMoveTo(int x, int y, const Unit* unit)
{
	return CanMoveToMask(x, y, TypeMovementMask(unit->Type));
}

#if 0

/**
**		Return the units field flags.
**		This flags are used to mark the field for this unit.
**
**		@param unit		Pointer to unit.
**
**		@return				Field flags to be set.
*/
global unsigned UnitFieldFlags(const Unit* unit)
{
	// FIXME: Should be moved into unittype structure, and allow more types.
	switch (unit->Type->UnitType) {
		case UnitTypeLand:				// on land
			return MapFieldLandUnit;
		case UnitTypeFly:				// in air
			return MapFieldAirUnit;
		case UnitTypeNaval:				// on water
			return MapFieldSeaUnit;
		default:
			DebugLevel1Fn("Were moves this unit?\n");
			return 0;
	}
}

/**
**		Return the unit type movement mask.
**				TODO: Should add this to unit-type structure.
**
**		@param type		Unit type pointer.
**
**		@return				Movement mask of unit type.
*/
global int TypeMovementMask(const UnitType* type)
{
	// FIXME: Should be moved into unittype structure, and allow more types.
	switch (type->UnitType) {
		case UnitTypeLand:				// on land
			return MapFieldLandUnit |
				MapFieldBuilding |		// already occuppied
				MapFieldWall |
				MapFieldRocks |
				MapFieldForest |		// wall,rock,forest not 100% clear?
				MapFieldCoastAllowed |
				MapFieldWaterAllowed |		// can't move on this
				MapFieldUnpassable;
		case UnitTypeFly:				// in air
			return MapFieldAirUnit;		// already occuppied
		case UnitTypeNaval:				// on water
			if( type->Transporter ) {
				return MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding |		// already occuppied
					MapFieldLandAllowed;		// can't move on this
					//| MapFieldUnpassable;		// FIXME: bug?
			}
			return MapFieldSeaUnit |
				MapFieldBuilding |		// already occuppied
				MapFieldCoastAllowed |
				MapFieldLandAllowed |		// can't move on this
				MapFieldUnpassable;
		default:
			DebugLevel1Fn("Were moves this unit?\n");
			return 0;
	}
}

/**
**		Return units movement mask.
**
**		@param unit		Unit pointer.
**
**		@return				Movement mask of unit.
*/
global int UnitMovementMask(const Unit* unit)
{
	return TypeMovementMask(unit->Type);
}

#endif

/**
**		Fixes initially the wood and seen tiles.
*/
global void PreprocessMap(void)
{
	int ix;
	int iy;
	MapField* mf;

	for (ix = 0; ix < TheMap.Width; ++ix) {
		for (iy = 0; iy < TheMap.Height; ++iy) {
			mf = TheMap.Fields + ix + iy * TheMap.Width;
			mf->SeenTile = mf->Tile;
		}
	}

	// it is required for fixing the wood that all tiles are marked as seen!
	for (ix = 0; ix < TheMap.Width; ++ix) {
		for (iy = 0; iy < TheMap.Height; ++iy) {
			MapFixWallTile(ix, iy);
			MapFixSeenWallTile(ix, iy);
		}
	}
}

/**
**		Convert viewport x coordinate to map tile x coordinate.
**
**		@param vp		Viewport pointer.
**		@param x		X coordinate into this viewport (in pixels, relative
**						to origin of Stratagus's window - not the viewport
**						itself!).
**
**		@return				X map tile coordinate.
*/
global int Viewport2MapX(const Viewport* vp, int x)
{
	int r;

	r = (x - vp->X) / TileSizeX + vp->MapX;
	return r < TheMap.Width ? r : TheMap.Width - 1;
}

/**
**		Convert viewport y coordinate to map tile y coordinate.
**
**		@param vp		Viewport pointer.
**		@param y		Y coordinate into this viewport (in pixels, relative
**						to origin of Stratagus's window - not the viewport
**						itself!).
**
**		@return				Y map tile coordinate.
*/
global int Viewport2MapY(const Viewport* vp, int y)
{
	int r;

	r = (y - vp->Y) / TileSizeY + vp->MapY;
	return r < TheMap.Height ? r : TheMap.Height - 1;
}

/**
**		Convert a map tile X coordinate into a viewport x pixel coordinate.
**
**		@param vp		Viewport pointer.
**		@param x		The map tile's X coordinate.
**
**		@return				X screen coordinate in pixels (relative
**					  to origin of Stratagus's window).
*/
global int Map2ViewportX(const Viewport* vp, int x)
{
	return vp->X + (x - vp->MapX) * TileSizeX;
}

/**
**		Convert a map tile Y coordinate into a viewport y pixel coordinate.
**
**		@param vp		Viewport pointer.
**		@param y		The map tile's Y coordinate.
**
**		@return				Y screen coordinate in pixels (relative
**					  to origin of Stratagus's window).
*/
global int Map2ViewportY(const Viewport* vp, int y)
{
	return vp->Y + (y - vp->MapY) * TileSizeY;
}

/**
**		Release info about a map.
**
**		@param info		MapInfo pointer.
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
		if (info->Filename) {
			free(info->Filename);
		}
		free(info);
	}
}

/**
**		Cleanup the map module.
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
	ReplayRevealMap = 0;

	DestroyMinimap();

	CleanMapFogOfWar();

	CleanPud();
}

//@}
