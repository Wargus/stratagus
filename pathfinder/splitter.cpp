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
/**@name splitter.c	-	Map splitter into regions. 	*/
//
//	(c) Copyright 1999-2003 by Ludovic Pollet
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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

#ifdef MAP_REGIONS

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stratagus.h"
#include "player.h"
#include "unit.h"
#include "map.h"

#include "pathfinder.h"

#include "splitter_local.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/


global int adjacents[8][2] = { {-1,-1}, {-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1}};

global RegionId* RegionMappingStorage;
global int* RegionTempStorage;
global int RegionCount;
global int RegionMax;
global int NextFreeRegion;
global RegionDefinition Regions[MaxRegionNumber];
global int MapSplitterInitialised;
global int ZoneNeedRefresh;


/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/


/**
**		Unassign a tile to a region.
**		Connections are not updated here
**
**		@param region		the region ID
**		@param x		X coord of the tile
**		@param y		Y coord of the tile
*/
global void RegionUnassignTile(RegionId region, int x, int y)
{
	RegionDefinition* adef;
	RegionSegment* cur;

	adef = Regions + region;

	// Unmap the tile
	RegionMapping(x, y) = NoRegion;

	// Remove from tile count
	adef->TileCount--;
	adef->SumX -= x;
	adef->SumY -= y;


	// Remove from segments
	cur = adef->FirstSegment;
	while (cur) {
			if (cur->Y == y && cur->MinX <= x && cur->MaxX >= x){
			if (x == cur->MinX) {
				cur->MinX++;
				if (cur->MinX > cur->MaxX){
					RegionDelSegment(adef, cur);
				}
			} else if (x == cur->MaxX) {
				cur->MaxX--;
				if (cur->MinX > cur->MaxX){
					RegionDelSegment(adef, cur);
				}
			} else {
				RegionAddSegment(adef, x + 1, cur->MaxX, y);
				cur->MaxX = x - 1;
			}
			break;
		}
		cur = cur->Next;
	}

	// update min & max
	adef->MinX = 0x7fffffff;
	adef->MinY = 0x7fffffff;
	adef->MaxX = -1;
	adef->MaxY = -1;

	cur = adef->FirstSegment;

	while (cur) {
		if (cur->Y > adef->MaxY) {
			adef->MaxY = cur->Y;
		}

		if (cur->Y < adef->MinY) {
			adef->MinY = cur->Y;
		}
		if (cur->MinX < adef->MinX) {
			adef->MinX = cur->MinX;
		}
		if (cur->MaxX > adef->MaxX) {
			adef->MaxX = cur->MaxX;
		}
		cur = cur->Next;
	}
}

/**
**		Assign a tile to a region.
**		Connections are not updated here
**
**		@param region		the region ID
**		@param x		X coord of the tile
**		@param y		Y coord of the tile
*/
global void RegionAssignTile(RegionId region, int x, int y)
{
	RegionDefinition* adef;
	RegionSegment* left;
	RegionSegment* right;
	RegionSegment* cur;

	RegionMapping(x,y) = region;
	adef = Regions + region;

	RegionUpdateMinMax(adef, x, y);

	adef->TileCount++;

	left = 0;
	right = 0;

	cur = adef->FirstSegment;

	while (cur) {
		if (cur->Y == y) {
			if (cur->MaxX == x - 1) {
				left = cur;
			}
			if (cur->MinX == x + 1) {
				right = cur;
			}
		}
		cur = cur->Next;
	}

	if (left && right) {
		left->MaxX = right->MaxX;
		RegionDelSegment(adef, right);
	} else if (left) {
		left->MaxX = x;
	} else if (right) {
		right->MinX = x;
	} else {
		// New segment.
		RegionAddSegment(adef, x, x, y);
	}
}

/**
**		Allocate a new region
**
**		@param iswater		Indicate if the region is water/sea/...
**		@return 		the new RegionID
*/
global RegionId NewRegion(int iswater)
{
	RegionId result;

	result = NextFreeRegion;

	Assert(NextFreeRegion < MaxRegionNumber);

	if (RegionMax <= result) {
		RegionMax = result + 1;
	}
	++RegionCount;

	Assert(!Regions[result].TileCount);

	Regions[result].TileCount = 0;
	Regions[result].IsWater = iswater;
	Regions[result].Connections = 0;
	Regions[result].ConnectionsCount = 0;
	Regions[result].ConnectionsNumber = 0;
	Regions[result].FirstSegment = 0;
	Regions[result].LastSegment = 0;
	Regions[result].NeedConnectTest = 0;
	Regions[result].MinX = 0x7ffffff;
	Regions[result].MinY = 0x7ffffff;
	Regions[result].MaxX = -1;
	Regions[result].MaxY = -1;
	Regions[result].SumX = 0;
	Regions[result].SumY = 0;
	Regions[result].Zone = -1;
	Regions[result].Dirty = -1;

	++NextFreeRegion;
	while (NextFreeRegion < RegionMax && Regions[NextFreeRegion].TileCount){
		++NextFreeRegion;
	}
	return result;
}

/**
**		Free a region
**
**		@param regid		The region to free
*/
global void RegionFree(RegionId regid)
{
	RegionSegment* cur;
	RegionSegment* next;

	--RegionCount;
	if (regid < NextFreeRegion) {
		NextFreeRegion = regid;
	}
	Regions[regid].TileCount = 0;
	if (Regions[regid].ConnectionsNumber) {
		free(Regions[regid].Connections);
		free(Regions[regid].ConnectionsCount);
	}
	Regions[regid].Connections = 0;
	Regions[regid].ConnectionsCount = 0;

	cur = Regions[regid].FirstSegment;
	while (cur) {
		next = cur->Next;
		free(cur);
		cur = next;
	}
	Regions[regid].FirstSegment = 0;
	Regions[regid].LastSegment = 0;
}

/**
**		Update connections for all regions (slow)
**
*/
global void UpdateConnections(void)
{
	int x;
	int y;
	RegionId reg;

	for (x = 0; x < TheMap.Width; ++x) {
			for (y = 0; y < TheMap.Height; ++y) {
			reg = RegionMapping(x, y);
			if (reg != NoRegion) {
				RegionUpdateConnection(reg, x, y, 1, 0);
			}
		}
	}
}

/**
**		Split region according to the content of the "TempStorage"
**		All tile with equal value will go in the same region
**
**		@param reg		The region to split
**		@param nbarea		The number of area
**		@param updateConnections		indicate if connection should be updated
*/
global void RegionSplitUsingTemp(RegionId reg, int nbarea, int updateConnections)
{
	RegionSegment* oldsegs;
	RegionSegment* seg;
	RegionDefinition** newregions;
	RegionId* newregionsid;
	int minx;
	int maxx;
	int initval;
	int x;
	int y;
	int i;
	int* tempptr;

	newregions = malloc(nbarea * sizeof(RegionDefinition*));
	newregionsid = malloc(nbarea * sizeof(RegionId));

	oldsegs = Regions[reg].FirstSegment;

	newregions[0] = Regions + reg;
	newregionsid[0] = reg;
	for (i = 1; i < nbarea; ++i) {
		newregionsid[i] = NewRegion(newregions[0]->IsWater);
		newregions[i] = Regions + newregionsid[i];
		newregions[i]->Zone = newregions[0]->Zone;
		newregions[i]->Dirty = newregions[0]->Dirty;
	}

	newregions[0]->FirstSegment = 0;
	newregions[0]->LastSegment = 0;
	newregions[0]->TileCount = 0;
	newregions[0]->MinX = 0x7ffffff;
	newregions[0]->MaxX = -1;
	newregions[0]->MinY = 0x7ffffff;
	newregions[0]->MaxY = -1;
	newregions[0]->SumX = 0;
	newregions[0]->SumY = 0;

	seg = oldsegs;

	while (seg) {
		minx = seg->MinX;
		maxx = seg->MinX;

		tempptr = RegionTempStorage + seg->Y * TheMap.Width + seg->MinX;

		while (minx <= seg->MaxX) {
			initval = *tempptr;
			Assert(initval != 0);

			while ((maxx < seg->MaxX) && (tempptr[1] == initval)) {
				tempptr++;
				maxx++;
			}

			RegionAddSegment(newregions[initval - 1], minx, maxx, seg->Y);
			RegionUpdateMinMax(newregions[initval - 1], minx, seg->Y);
			RegionUpdateMinMax(newregions[initval - 1], maxx, seg->Y);
			newregions[initval - 1]->TileCount += maxx - minx + 1;

			y = seg->Y;
			for (x = minx; x <= maxx; ++x) {
				RegionMapping(x,y) = newregionsid[initval - 1];
			}

			++maxx;
			++tempptr;
			minx = maxx;
		}

		seg = seg->Next;
	}

	while (oldsegs) {
		seg = oldsegs->Next;
		free(oldsegs);
		oldsegs = seg;
	}

	if (!updateConnections) {
		free(newregions);
		free(newregionsid);
		return;
	}

	for (i = 0; i < nbarea; ++i) {
		RegionRescanAdjacents(newregionsid[i]);
	}

	free(newregions);
	free(newregionsid);
}

/**
**		Join to region into only one. Either a or b is destroyed
**
**		@param a		One of the two regions
**		@param b		One of the two regions
*/
global void RegionJoin(RegionId a, RegionId b)
{
	RegionSegment* cur;
	RegionId tmp;
	RegionId* mapptr;
	int i;

	Assert(Regions[a].IsWater == Regions[b].IsWater);
	if (a > b) {
		tmp = a;
		a = b;
		b = tmp;
	}

	cur = Regions[b].FirstSegment;
	while (cur) {
		mapptr = RegionMappingStorage + cur->MinX + cur->Y * TheMap.Width;
		for ( i = cur->MaxX - cur->MinX + 1; i > 0; --i) {
			*(mapptr++) = a;
		}
		RegionAppendSegment(Regions + a, cur->MinX, cur->MaxX, cur->Y);
		cur = cur->Next;
	}
	Regions[a].TileCount += Regions[b].TileCount;
	Regions[a].SumX += Regions[b].SumX;
	Regions[a].SumY += Regions[b].SumY;
	Regions[a].Dirty = 0;
	Regions[a].MinX = (Regions[a].MinX < Regions[b].MinX) ? Regions[a].MinX : Regions[b].MinX;
	Regions[a].MinY = (Regions[a].MinY < Regions[b].MinY) ? Regions[a].MinY : Regions[b].MinY;
	Regions[a].MaxX = (Regions[a].MaxX > Regions[b].MaxX) ? Regions[a].MaxX : Regions[b].MaxX;
	Regions[a].MaxY = (Regions[a].MaxY > Regions[b].MaxY) ? Regions[a].MaxY : Regions[b].MaxY;

	// Update connections : a receive all that b has
	while (Regions[b].ConnectionsNumber) {
		if (Regions[b].Connections[0] != a) {
			RegionAddBidirConnection(a, Regions[b].Connections[0], Regions[b].ConnectionsCount[0]);
		}
		RegionAddBidirConnection(b, Regions[b].Connections[0], -Regions[b].ConnectionsCount[0]);
	}

	RegionFree(b);
}

/**
**		Split a region in two parts
**
**		@param regid		the region to broke
**		@param updateConnections		indicate if connection should be updated as well
*/
global void RegionSplit(RegionId regid, int updateConnections)
{
	RegionDefinition* adef;
	int tileleft;
	int x;
	int y;
	int obstacle;
	int nv_obstacle;
	int done;
	int erodelevel;
	int blocker;
	int oldZoneNeedRefresh;
	int i;
	CircularFiller fillers[2];		// We have 2 concurrent floodfiller

	oldZoneNeedRefresh = ZoneNeedRefresh;

	adef = Regions + regid;

	Assert(adef->TileCount > 1);

	RegionTempStorageAllocate();

	// Start filling the region with 0
	RegionTempStorageFillRegion(adef, 0);

	tileleft = adef->TileCount;

	obstacle = 0;
	erodelevel = 2;
	while ((erodelevel < 8) && (obstacle <= tileleft / 4)) {
			if (erodelevel == 2) {
			// Mark limits points for putting obstacle
			nv_obstacle = RegionTempStorageMarkObstacle(regid, tileleft - 10 - obstacle, ++erodelevel);
			} else {
			// Make existing obstacle bigger
			nv_obstacle = RegionTempStorageEmbossObstacle(regid, tileleft - 10 - obstacle, erodelevel++);
		}
		if (!nv_obstacle) {
			// Nothing marked, undo & time to break
			erodelevel--;
			break;
		}
		obstacle += nv_obstacle;
	}

	// Find two correct starting place for flood filling
	if (adef->MaxX - adef->MinX > adef->MaxY - adef->MinY) {
		RegionFindPointOnX(adef, adef->MinX, &x, &y);
		Assert(!RegionTempStorage[x + TheMap.Width * y]);
		CircularFillerInit(fillers, regid, x, y, 1);

		RegionFindPointOnX(adef, adef->MaxX, &x, &y);
		Assert(!RegionTempStorage[x + TheMap.Width * y]);
		CircularFillerInit(fillers + 1, regid, x, y, 2);
	} else {
		RegionFindPointOnY(adef, adef->MinY, &x, &y);
		Assert(!RegionTempStorage[x + TheMap.Width * y]);
		CircularFillerInit(fillers, regid, x, y, 1);

		RegionFindPointOnY(adef, adef->MaxY, &x, &y);
		Assert(!RegionTempStorage[x + TheMap.Width * y]);
		CircularFillerInit(fillers + 1, regid, x, y, 2);
	}

	tileleft -= 2;

	while (tileleft) {
		// Search with the two
		done = 0;
		blocker = -1;
		while (blocker == -1) {
			i = (fillers[0].NextOne < fillers[1].NextOne ? 0 : 1);
			if (CircularFillerStep(fillers + i)) {
				++done;
			} else {
				blocker = i;
			}
		}

		// Other take advance
		while (CircularFillerStep(fillers + 1 - blocker)) {
			++done;
		}

		// Need to unmark ?
		if (done < tileleft) {
			Assert(erodelevel >= 3);

			RegionTempStorageUnmarkPoints(regid, erodelevel--);

			// Restart both fillers
			fillers[0].NextOne = 0;
			fillers[1].NextOne = 0;
		}

		tileleft -= done;
	}

	CircularFillerDone(fillers);
	CircularFillerDone(fillers + 1);

	RegionSplitUsingTemp(regid, 2, updateConnections);

	RegionTempStorageFree();
	ZoneNeedRefresh = oldZoneNeedRefresh || ! updateConnections;
}

/**
**		Check that the given region is 8 - connex
**		( all its tiles are reachable )
**
**		@param reg		the region ID
*/
global void RegionCheckConnex(RegionId reg)
{
	CircularFiller filler;
	int nbarea;
	int tilesleft;
	RegionSegment* seg;

	RegionTempStorageAllocate();

	RegionTempStorageFillRegion(Regions + reg, 0);

	nbarea = 0;

	Regions[reg].NeedConnectTest = 0;

	tilesleft = Regions[reg].TileCount;
	seg = Regions[reg].FirstSegment;
	while (seg) {
		if (!RegionTempStorage[seg->MinX + TheMap.Width * seg->Y]) {
			nbarea++;
			CircularFillerInit(&filler, reg, seg->MinX, seg->Y, nbarea);
			--tilesleft;
			while (CircularFillerStep(&filler)) {
				--tilesleft;
			}
			CircularFillerDone(&filler);
			if (!tilesleft) {
				break;
			}
		}
		seg = seg->Next;
	}

	if (nbarea > 1) {
		// RegionDebugAllConnexions();
		Regions[reg].Dirty += 10;
		RegionSplitUsingTemp(reg, nbarea, 1);
		ZoneNeedRefresh = 1;
		// RegionDebugAllConnexions();
	}

	RegionTempStorageFree();
}

/**
**		Called when a tile should no more belong to any regions
**
**		@param x		x position of the tile
**		@param y		y position of the tile
*/
local void MapSplitterTileOccuped(int x, int y) {
	RegionId reg;
	int tx;
	int ty;
	int pathcount;
	int hasadjacent[8];
	int i;
	int lastival;

	reg = RegionMapping(x, y);
	if (reg == NoRegion) {
		return;
	}

	Regions[reg].Dirty++;

	RegionUnassignTile(reg, x, y);

	RegionUpdateConnection(reg, x, y, -1, 1);

	if (!Regions[reg].TileCount) {
		RegionFree(reg);
		return;
	}

	if (Regions[reg].TileCount == 1) {
		// No problem of Connection here
		return;
	}

	// Count different path from the removed cell.
	for (i = 0; i < 8; ++i){
		tx = x + adjacents[i][0];
		ty = y + adjacents[i][1];

		hasadjacent[i] = (InMap(tx, ty) && RegionMapping(tx, ty) == reg);
	}

	pathcount = 0;
	lastival = hasadjacent[7];
	for (i = 0; i < 8; ++i) {
		if (lastival && !hasadjacent[i]) {
			++pathcount;
		}
		lastival = hasadjacent[i];
	}

	if (pathcount <= 1) {
		// No problem of disconnection
		return;
	}

	// Here we'll need to flood fill the region to be sure...
	Regions[reg].NeedConnectTest = 1;
}

/**
**		Add a rectangle of tiles to region mapping. Called when map change
**
**		@param x0		x0 coord of the changed rectangle
**		@param y0		y0 coord of the changed rectangle
**		@param x1		x1 coord of the changed rectangle
**		@param y1		y1 coord of the changed rectangle
*/
global void MapSplitterTilesCleared(int x0, int y0, int x1, int y1) {
	static int directions[5][2] = {{1,0},{0,1},{-1,0},{0,-1},{0,0}};
	int x;
	int y;
	int i;
	RegionId regid;
	RegionId oppreg;
	RegionId adjacents[256];
	int adjacentsCount[256];
	int adjacentsNb;
	int bestAdjacent;
	int dir;
	int iswater;

	if (!MapSplitterInitialised) {
		return;
	}

#ifdef DEBUG
	for (y = y0; y <= y1; ++y) {
		for (x = x0; x <= x1; ++x) {
			if (RegionMapping(x, y) != NoRegion) {
				for (y = y0; y <= y1; ++y) {
					for (x = x0; x <= x1; ++x) {
						if (RegionMapping(x, y) == NoRegion) {
							MapSplitterTilesCleared(x, y, x, y);
						}
					}
				}

				return;
			}
		}
	}
#endif

	iswater = TileIsWater(x0, y0) != 0;

	// Find adjacent regions
	x = x0 - 1;
	y = y0 - 1;
	dir = -1;
	adjacentsNb = 0;
	do {
		// Check x,y
		if (InMap(x, y)) {
			oppreg = RegionMapping(x, y);
			if (oppreg != NoRegion && Regions[oppreg].IsWater == iswater) {
				for (i = 0; i < adjacentsNb; ++i) {
					if (adjacents[i] == oppreg) {
							adjacentsCount[i]++;
							oppreg = NoRegion;
							break;
					}
				}
				if (oppreg != NoRegion) {
					adjacents[adjacentsNb] = oppreg;
					adjacentsCount[adjacentsNb] = 1;
					adjacentsNb++;
				}
			}
		}

		// Increment x, y
		if ((x == x0 - 1 || x == x1 + 1) && (y == y0 - 1 || y == y1 + 1)) {
			dir++;
		}

		x += directions[dir][0];
		y += directions[dir][1];
	} while (dir < 4);

	bestAdjacent = -1;
	for (i = 0; i < adjacentsNb; ++i) {
		if (bestAdjacent == -1 || adjacentsCount[i] > adjacentsCount[bestAdjacent]) {
			bestAdjacent = i;
		}
	}

	// Create new region if no connection, or connection to big area is only 1 cell
	if (bestAdjacent == -1 || (adjacentsCount[bestAdjacent] < 2 && (x0 != x1 || y0 != y1))) {
		// create new region
		regid = NewRegion(TileIsWater(x0, y0));
	} else {
		// Find most interesting region
		regid = adjacents[bestAdjacent];
	}

	for (y = y0; y <= y1; ++y) {
		for (x = x0; x <= x1; ++x) {
			RegionAssignTile(regid, x, y);
			RegionUpdateConnection(regid, x, y, 1, 1);
			Regions[regid].Dirty++;
		}
	}
}

/**
**		Remove a rectangle of tiles from region mapping. Called when map change
**
**		@param x0		x0 coord of the changed rectangle
**		@param y0		y0 coord of the changed rectangle
**		@param x1		x1 coord of the changed rectangle
**		@param y1		y1 coord of the changed rectangle
*/
global void MapSplitterTilesOccuped(int x0, int y0, int x1, int y1)
{
	int x;
	int y;

	if (!MapSplitterInitialised) {
		return;
	}

	for (y = y0; y <= y1; ++y) {
		for (x = x0; x <= x1; ++x) {
			MapSplitterTileOccuped(x, y);
		}
	}
}

/**
**		Decide if region should be broken, regarding size & nb of tiles
**
*/
local int ShouldBreakRegion(int x0, int y0, int x1, int y1, int tilecount, int hardlimit)
{
	int sx;
	int sy;
	int square;

	// Don't break very small cells
	if (tilecount < 48) {
		return 0;
	}

	// Break very big ones
	if (tilecount > (hardlimit ? 512 : 1024)) {
		return 1;
	}

	sx = x1 - x0 + 1;
	sy = y1 - y0 + 1;
	square = (sy > sx ? sy : sx);

	return 10 * tilecount < 6 * square * square;
}

/**
**		Extend A segment, fill it.
**
*/
local void FindHExtent(int x, int y, int* vx0, int* vx1, int water)
{
	int x0;
	int x1;

	if (x > 0) {
			x0 = x;
			x1 = x - 1;
	} else {
		x0 = x + 1;
		x1 = x;
	}

	// Try extending to the left
	while ((x0 > 0) && (TileMappable(x0 - 1, y)) &&
			((TileIsWater(x0 - 1, y) != 0) == water)) {
		--x0;
	}

	// Try extending to the right
	while ((x1 + 1 < TheMap.Width && (TileMappable(x1 + 1, y)) &&
			((TileIsWater(x1 + 1, y) != 0) == water))) {
		++x1;
	}

	*vx0 = x0;
	*vx1 = x1;
}

/**
**		Flood fill a region in the mapping area
*/
local void RegionFloodFill(int x0, int x1, int starty, int RegId, int IsWater)
{
	int subx0;
	int subx1;
	int x;
	int y;
	int i;

	Assert(x0 <= x1);
	Assert(IsWater == 0 || IsWater == 1);

	y = starty;

	for (x = x0; x <= x1; ++x) {
		Assert(TileIsWater(x, y) == IsWater);
		RegionAssignTile(RegId, x, y);
	}

	// Try in yinc dir
	for (i = 0; i < 2; ++i){
		y = starty + (i ? -1 : 1);
			for (x = x0 - 1;x <= x1 + 1; ++x) {
			if (!InMap(x, y)) {
					continue;
			}

			if (!TileMappable(x, y)) {
				continue;
			}

			if (TileIsWater(x, y) != IsWater) {
				continue;
			}

			if (RegionMapping(x, y) != NoRegion) {
				continue;
			}

			FindHExtent(x, y, &subx0, &subx1, IsWater);
			Assert(TileIsWater(subx0,y) == IsWater);
			Assert(TileIsWater(subx1,y) == IsWater);
			RegionFloodFill(subx0, subx1, y, RegId, IsWater);

			x = subx1;
		}
	}
}

/**
**		Initialise the region mapping ( map tile => regions )
**		Need an already initialised map to work correctly
*/
global void InitaliseMapping(void)
{
	int found;
	int i;
	int total;
	int x;
	int y;
	int x0;
	int x1;
	int CurrentIsWater;

	for (i = 0; i < MaxRegionNumber; ++i) {
		Regions[i].TileCount = 0;
		Regions[i].Connections = 0;
		Regions[i].ConnectionsCount = 0;
		Regions[i].ConnectionsNumber = 0;
		Regions[i].FirstSegment = 0;
		Regions[i].LastSegment = 0;
	}

	total = TheMap.Width * TheMap.Height;
	for (i = 0; i < total; ++i) {
		RegionMappingStorage[i] = NoRegion;
	}

	for (y = 0; y < TheMap.Height; ++y) {
			for (x = 0; x < TheMap.Width; ++x) {
			if (!TileMappable(x, y)) {
				continue;
			}

			if (RegionMapping(x, y) != NoRegion) {
				continue;
			}

			CurrentIsWater = TileIsWater(x, y);
			FindHExtent(x, y, &x0, &x1, CurrentIsWater);

			RegionFloodFill(x0, x1, y, NewRegion(CurrentIsWater), CurrentIsWater);
			x = x1;
		}
	}
	UpdateConnections();

	RegionDebugAllConnexions();
	RegionDebugWater();

	do {
		// FIXME : detect regions with holes not well connected
		found = 0;
		for (i = 0; i < RegionMax; ++i) {
			// Get region size
			x = Regions[i].MaxX - Regions[i].MinX + 1;
			y = Regions[i].MaxY - Regions[i].MinY + 1;

			// Split region which are big or are not square at all...
			if ((Regions[i].TileCount > 1024) ||
						(Regions[i].TileCount > 64 &&
					(x > y ? x : y) * (x > y ? x : y) > 3 * Regions[i].TileCount)) {
				RegionSplit(i, 1);
				// RegionDebugAllConnexions();
				found = 1;
				break;
			}
		}
	} while (found);
}

/**
**		Find a point of connexion between two zone.
**		The point closer to (refx,refy) in (a) is returned
**
**		@param a		a zone number
**		@param b		the other zone number
**		@param refx		Search closest to (refx,refy)
**		@param refy		Search closest to (refx,refy)
**		@param rsltx		Will hold result X
**		@param rsltx		Will hold result Y
*/
global void ZoneFindConnexion(int a, int b, int refx, int refy, int* rsltx, int* rslty)
{
	int oppzone;
	RegionId oppregion;
	RegionSegment* rg;
	int x;
	int y;
	int tx;
	int ty;
	int i;
	int j;
	int k;
	int dst;
	int bestdst;

	bestdst = -1;
	for (i = 0; i < RegionMax; ++i) {
		if (Regions[i].Zone == a) {
			for (j = 0; j < Regions[i].ConnectionsNumber; ++j) {
				oppregion = Regions[i].Connections[j];
				oppzone = Regions[oppregion].Zone;
				if (oppzone != b) {
					continue;
				}

				// OK, find a point in region i, adjacent to oppregion
				rg = Regions[i].FirstSegment;
				while (rg) {
					y = rg->Y;
					for (x = rg->MinX; x <= rg->MaxX; ++x) {
						for (k = 0; k < 8; ++k) {
							tx = x + adjacents[k][0];
							ty = y + adjacents[k][1];

							if (!InMap(tx,ty)) {
								continue;
							}
							oppregion = RegionMapping(tx, ty);
							if (oppregion == NoRegion || Regions[oppregion].Zone != b) {
								continue;
							}
							dst = (x - refx) * (x - refx) + (y - refy) * (y - refy);
							if (bestdst == -1 || dst < bestdst){
								*rsltx = x;
								*rslty = y;
								bestdst = dst;
							}
						}
					}
					rg = rg->Next;
				}
				break;
			}
		}
	}

	Assert(bestdst != -1);
}

/**
**		Refresh connection between zones
*/
local void RefreshZones(void)
{
	int* regions_stack;
	RegionId regid;
	RegionId adjid;
	int stack_size;
	int stack_ptr;
	int i;
	int j;
	int zoneid;

	Assert(RegionCount);
	regions_stack = malloc(RegionCount * sizeof(int));

	for (i = 0; i < RegionMax; ++i) {
		Regions[i].Zone = -1;
	}

	zoneid = 0;

	i = 0;
	do {
		while (i < RegionMax && ((!Regions[i].TileCount) || Regions[i].Zone != -1)) {
			++i;
			}

		if (i == RegionMax) {
			free(regions_stack);
			return;
		}

		Regions[i].Zone = zoneid;

		stack_ptr = 0;
		regions_stack[0] = i;
		stack_size = 1;

		while (stack_ptr < stack_size) {
			regid = regions_stack[stack_ptr++];
			for (j = 0; j < Regions[regid].ConnectionsNumber; ++j) {
				adjid = Regions[regid].Connections[j];
				if (Regions[adjid].Zone != -1) {
					continue;
				}

				if (Regions[adjid].IsWater != Regions[regid].IsWater) {
					continue;
				}
				regions_stack[stack_size++] = adjid;
				Regions[adjid].Zone = zoneid;
			}
		}

		++zoneid;
	} while (1);

	// Should never get here, but just in case
	free(regions_stack);
}

/**
**		Allocate space for tile=>region mapping
*/
local void AllocateMapping(void)
{
	int total;

	total = TheMap.Width * TheMap.Height;
	RegionMappingStorage = (RegionId*) malloc(sizeof(RegionId) * total);
	NextFreeRegion = 0;
	RegionCount = 0;
	RegionMax = 0;
}

/**
** 		Initialise all data structures of the MapSplitter
**
*/
global void MapSplitterInit(void)
{
	MapSplitterInitialised = 1;

	AllocateMapping();
	InitaliseMapping();
	RefreshZones();
	ZoneNeedRefresh = 0;

	RegionDebugAllConnexions();
}

/**
**		Free all structure owned by the MapSplitter
**
*/
global void MapSplitterClean(void)
{
	// FIXME : free !
	MapSplitterInitialised = 0;
}

/**
**		Called each cycle to maintain correctness of the mapping
**
*/
global void MapSplitterEachCycle(void)
{
	int k;
	int x0;
	int y0;
	int x1;
	int y1;
	RegionId i;
	RegionId j;

	Assert(MapSplitterInitialised);

	// Check for connection in regions
	for (i = 0; i < RegionMax; ++i) {
		if (Regions[i].TileCount && Regions[i].NeedConnectTest) {
			RegionCheckConnex(i);
		}
	}

	// split big & diform regions
	if (RegionCount < MaxRegionNumber / 4) {
		// Try to split regions
		for (i = 0; i < RegionMax; ++i) {
			if (Regions[i].Dirty && ShouldBreakRegion(Regions[i].MinX,Regions[i].MinY,
					Regions[i].MaxX,Regions[i].MaxY,Regions[i].TileCount,1)) {

				RegionSplit(i, 1);

				// RegionDebugAllConnexions();
				if (RegionCount >= MaxRegionNumber / 4) {
					break;
				}
			}
		}
	}

	// find smalls & really connected regions
	for (i = 0; i < RegionMax; ++i) {
		// Try to complete this region
		if (Regions[i].Dirty && Regions[i].TileCount && Regions[i].TileCount < 1024) {
			for (k = 0; k < Regions[i].ConnectionsNumber; ++k) {
				j = Regions[i].Connections[k];


				if (Regions[i].TileCount + Regions[j].TileCount > 1024) {
					continue;
				}


				if (Regions[i].IsWater != Regions[j].IsWater) {
					continue;
				}

				// ConnectionsCount == ~ 3 * n°of tile connected
				// (avoid to connect region with only few adjacents cells)
				if ((Regions[i].TileCount > 256 && Regions[j].TileCount> 256) &&
					Regions[i].ConnectionsCount[k] / 3 < isqrt(Regions[i].TileCount) / 2 &&
					Regions[i].ConnectionsCount[k] / 3 < isqrt(Regions[j].TileCount) / 2) {
					continue;
				}

				x0 = (Regions[i].MinX < Regions[j].MinX) ? Regions[i].MinX : Regions[j].MinX;
				y0 = (Regions[i].MinY < Regions[j].MinY) ? Regions[i].MinY : Regions[j].MinY;
				x1 = (Regions[i].MaxX > Regions[j].MaxX) ? Regions[i].MaxX : Regions[j].MaxX;
				y1 = (Regions[i].MaxY > Regions[j].MaxY) ? Regions[i].MaxY : Regions[j].MaxY;

				if (!ShouldBreakRegion(x0, y0, x1, y1,
							Regions[i].TileCount + Regions[j].TileCount, 1)) {
					RegionJoin(i, j);
					// RegionDebugAllConnexions();
					if (!Regions[i].TileCount) {
						break;
					}
				}
			}
		}
		Regions[i].Dirty = 0;
	}

	ClearZoneNeedRefresh();
}

/**
**		Can the unit 'src' reach the place x,y.
**
**		@param src		Unit for the path.
**		@param x		Map X tile position.
**		@param y		Map Y tile position.
**		@param w		Width of Goal
**		@param h		Height of Goal
**		@param range		Range to the tile.
**
**		@return				Distance to place.
*/
global int PlaceReachable(Unit* src, int goal_x, int goal_y, int w, int h, int minrange, int maxrange)
{
	static ZoneSet source = {0};
	static ZoneSet dest = {0};

	if (src->Type->UnitType == UnitTypeFly) {
		return 1;
	}

	ZoneSetClear(&source);
	ZoneSetClear(&dest);

	ZoneSetAddUnitZones(&source, src);
	ZoneSetAddGoalZones(&dest, src, goal_x, goal_y, w, h, minrange, maxrange);

	ZoneSetIntersect(&source, &dest);
	return source.ZoneCount != 0;
}

/**
**		Check if zone connections need a refresh & do it
*/
global void ClearZoneNeedRefresh(void)
{
	if (ZoneNeedRefresh) {
		RefreshZones();
		ZoneNeedRefresh = 0;
	}
}

#endif // MAP_REGIONS
//@}
